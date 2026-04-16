#include "EngineLaser.h"
#include <QDebug>

#define MINIAUDIO_IMPLEMENTATION
#include "lib/miniaudio.h"

const int EngineLaser::MIDI_BASE;

#define TSF_IMPLEMENTATION
#include "lib/tsf.h"

// Offsets MIDI depuis Do4 (60)
// Blanches : Do Re Mi Fa Sol La Si Do Re Mi Fa Sol
static const int MIDI_BLANC[] = {0,2,4,5,7,9,11,12,14,16,17,19};
// Noires   : Do# Re# -  Fa# Sol# La# -  Do# Re# -  Fa# -
static const int MIDI_NOIR[]  = {1,3,-1,6,8,10,-1,13,15,-1,18,-1};

EngineLaser::EngineLaser(QObject *parent, QString& soundFontPath) : QObject(parent)
{
    // Assignation par défaut : laser i → note i-1 (Do, Re, Mi, Fa, Sol, La)
    for (int i = 1; i <= 6; i++)
        m_laserMidiNote[i] = MIDI_BASE + MIDI_BLANC[i - 1];

    // ── Init audio ──
    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = 2;
    cfg.sampleRate = 44100;
    cfg.dataCallback = EngineLaser::audioCallback;
    cfg.pUserData = this;

    if (ma_device_init(nullptr, &cfg, &m_device) != MA_SUCCESS) {
        qWarning() << "EngineLaser: impossible d'initialiser l'audio";
        return;
    }

    ma_mutex_init(&m_mutex);

    // ── Charger le SoundFont ──
    if(soundFontPath != nullptr)
        m_tsf = tsf_load_filename(soundFontPath.toLocal8Bit().constData());
    else
        m_tsf = tsf_load_filename("florestan-subset.sf2");

    if (!m_tsf) {
        qWarning() << "EngineLaser: SoundFont introuvable !";
        qWarning() << "Placez le fichier dans le dossier de build.";
        ma_device_uninit(&m_device);
        return;
    }

    tsf_set_output(m_tsf, TSF_STEREO_INTERLEAVED, 44100, 0);

    if (ma_device_start(&m_device) != MA_SUCCESS) {
        qWarning() << "EngineLaser: impossible de démarrer la lecture";
        ma_device_uninit(&m_device);
        return;
    }

    m_audioOk = true;
    qDebug() << "EngineLaser: audio OK —"
             << tsf_get_presetcount(m_tsf) << "instruments disponibles";
}

EngineLaser::~EngineLaser()
{
    stopMidi();
    if (m_audioOk) {
        ma_device_uninit(&m_device);
        ma_mutex_uninit(&m_mutex);
    }
    if (m_tsf)
        tsf_close(m_tsf);
}

// ─────────────────────────────────────────────
void EngineLaser::audioCallback(ma_device* dev, void* out,
                                const void* /*in*/, ma_uint32 frames)
{
    EngineLaser *self = static_cast<EngineLaser*>(dev->pUserData);
    ma_mutex_lock(&self->m_mutex);
    tsf_render_float(self->m_tsf, (float*)out, (int)frames, 0);
    ma_mutex_unlock(&self->m_mutex);
}

// ─────────────────────────────────────────────
void EngineLaser::chargerInstrument(int idInstrument)
{
    if (!m_tsf) return;

    if (idInstrument < tsf_get_presetcount(m_tsf)) {
        tsf_note_off_all(m_tsf);
        // mute all sound before changing instrument
        m_instrument = idInstrument;
        return;
    }
    m_instrument = 0;
    qDebug() << "Instrument non trouvé, fallback 0";
}

QStringList EngineLaser::getInstrumentsDisponibles() const
{
    QStringList liste;
    if (!m_tsf) return liste;
    int count = tsf_get_presetcount(m_tsf);
    for (int i = 0; i < count; i++)
        liste << QString("%1 - %2").arg(i).arg(tsf_get_presetname(m_tsf, i));
    return liste;
}

// ─────────────────────────────────────────────
int EngineLaser::noteIndexToMidi(int noteIndex, bool estNoire)
{
    if (noteIndex < 0 || noteIndex >= 12) return MIDI_BASE;
    if (estNoire) {
        int off = MIDI_NOIR[noteIndex];
        return (off < 0) ? MIDI_BASE : MIDI_BASE + off;
    }
    return MIDI_BASE + MIDI_BLANC[noteIndex];
}

void EngineLaser::setNoteIndex(int laserId, int noteIndex, bool estNoire)
{
    // AJOUTE : stoppe l'ancienne note avant de changer
    int ancienMidi = m_laserMidiNote.value(laserId, MIDI_BASE);
    ma_mutex_lock(&m_mutex);
    tsf_note_off(m_tsf, m_instrument, ancienMidi);
    ma_mutex_unlock(&m_mutex);

    m_laserMidiNote[laserId] = noteIndexToMidi(noteIndex, estNoire);
    qDebug() << "Laser" << laserId << "-> MIDI" << m_laserMidiNote[laserId];
}

int EngineLaser::midiNoteForLaser(int laserId)
{
    return m_laserMidiNote.value(laserId, MIDI_BASE);
}

// ─────────────────────────────────────────────
void EngineLaser::jouerNote(int laserId)
{
    if (!m_audioOk || !m_tsf) return;
    int midi = m_laserMidiNote.value(laserId, MIDI_BASE);
    ma_mutex_lock(&m_mutex);
    tsf_note_on(m_tsf, m_instrument, midi, m_volume);
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::stopperNote(int laserId)
{
    if (!m_audioOk || !m_tsf) return;
    int midi = m_laserMidiNote.value(laserId, MIDI_BASE);
    ma_mutex_lock(&m_mutex);
    tsf_note_off(m_tsf, m_instrument, midi);
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::jouerNoteDirecte(int noteIndex, bool estNoire)
{
    if (!m_audioOk || !m_tsf) return;
    int midi = noteIndexToMidi(noteIndex, estNoire);
    ma_mutex_lock(&m_mutex);
    tsf_note_on(m_tsf, m_instrument, midi, m_volume);
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::stopperNoteDirecte(int noteIndex, bool estNoire)
{
    if (!m_audioOk || !m_tsf) return;
    int midi = noteIndexToMidi(noteIndex, estNoire);
    ma_mutex_lock(&m_mutex);
    tsf_note_off(m_tsf, m_instrument, midi);
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::setVolume(float niveau)
{
    m_volume = niveau;
    if (m_tsf) {
        ma_mutex_lock(&m_mutex);
        tsf_set_volume(m_tsf, niveau);
        ma_mutex_unlock(&m_mutex);
    }
}

// ─────────────────────────────────────────────
bool EngineLaser::initMidi(unsigned int id)
{
    try {
        m_midiIn = new RtMidiIn();
        unsigned int nPorts = m_midiIn->getPortCount();
        if (nPorts == 0) {
            qWarning() << "Aucun port MIDI disponible";
            delete m_midiIn;
            m_midiIn = nullptr;
            return false;
        }
        if(id > nPorts)
            id = 0;

        m_midiIn->openPort(id);
        m_midiIn->setCallback(&EngineLaser::midiCallback, this);
        m_midiIn->ignoreTypes(false, false, false);
        qDebug() << "MIDI ouvert:"
                 << QString::fromStdString(m_midiIn->getPortName(id));
        return true;

    } catch (RtMidiError &e) {
        qWarning() << "Erreur MIDI:"
                   << QString::fromStdString(e.getMessage());
        return false;
    }
}

QStringList EngineLaser::getMidiPorts()
{
    QStringList liste;
    RtMidiIn* midiIn = new RtMidiIn();

    unsigned int nPorts = midiIn->getPortCount();

    if (nPorts == 0) {
        delete midiIn;
        midiIn = nullptr;
        return liste;
    }

    for (unsigned int i = 0; i < nPorts; i++)
        liste << QString::fromStdString(midiIn->getPortName(i));

    delete midiIn;
    midiIn = nullptr;

    return liste;
}

void EngineLaser::stopMidi()
{
    if (m_midiIn) {
        m_midiIn->cancelCallback();
        m_midiIn->closePort();
        delete m_midiIn;
        m_midiIn = nullptr;
    }
}

void EngineLaser::midiCallback(double /*dt*/,
                               std::vector<unsigned char>* msg,
                               void* userData)
{
    EngineLaser *self = static_cast<EngineLaser*>(userData);
    if (!self || !self->m_tsf) return;
    if (msg->size() < 3) return;

    unsigned char status = msg->at(0);
    int note     = msg->at(1);
    int velocity = msg->at(2);

    if (status == 0x90 && velocity > 0) {
        ma_mutex_lock(&self->m_mutex);
        tsf_note_on(self->m_tsf, self->m_instrument, note, self->m_volume);
        ma_mutex_unlock(&self->m_mutex);
        emit self->noteRecueMidi(note, true);
    }
    else if (status == 0x80 || (status == 0x90 && velocity == 0)) {
        ma_mutex_lock(&self->m_mutex);
        tsf_note_off(self->m_tsf, self->m_instrument, note);
        ma_mutex_unlock(&self->m_mutex);
        emit self->noteRecueMidi(note, false);
    }
}

void EngineLaser::loadSoundFont(QString& fileName)
{
    tsf_close(m_tsf); // Free the memory used by the current SoundFont

    m_tsf = tsf_load_filename(fileName.toLocal8Bit().constData());
    if (!m_tsf) {
        qWarning() << "EngineLaser: SoundFont introuvable !";
        qWarning() << "Chargement du fichier par default.";
        m_tsf = tsf_load_filename("florestan-subset.sf2");
        return;
    }
}
