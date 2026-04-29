#include "EngineLaser.h"
#include <QDebug>

#define MINIAUDIO_IMPLEMENTATION
#include "lib/miniaudio.h"

#define TSF_IMPLEMENTATION
#include "lib/tsf.h"

EngineLaser::EngineLaser(QObject *parent) : QObject(parent)
{
    m_audioOk = false;
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

bool EngineLaser::initEngine(QString& soundFontPath, int idAudioOut)
{
    if (m_audioOk) {
        ma_device_uninit(&m_device);
        ma_mutex_uninit(&m_mutex);
        m_audioOk = false;
    }
    if (m_tsf)
        tsf_close(m_tsf);

    ma_result result = ma_context_init(NULL, 0, NULL, &m_context);
    if (result != MA_SUCCESS)
    {
        return 0;
    }

    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;
    result = ma_context_get_devices(&m_context, &pPlaybackDeviceInfos, &playbackDeviceCount, NULL, NULL);
    if (result != MA_SUCCESS)
    {
        ma_context_uninit(&m_context);
        return 0;
    }

    if(idAudioOut == -1 || idAudioOut >= playbackDeviceCount)
    {
        idAudioOut = 0;
        for (ma_uint32  iDevice = 0; iDevice < playbackDeviceCount; ++iDevice)
        {
            if(pPlaybackDeviceInfos[iDevice].isDefault)
                idAudioOut = iDevice; // save default device id
        }
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID       = &pPlaybackDeviceInfos[idAudioOut].id;
    config.playback.format          = ma_format_f32;
    config.playback.channels        = 2;
    config.sampleRate               = 44100;
    config.dataCallback             = EngineLaser::audioCallback;
    config.pUserData                = this;
    // TODO : find the optimal values
    config.periodSizeInMilliseconds = 1; // delay ~14 ms
    config.periods                  = 2;

    if (ma_device_init(&m_context, &config, &m_device) != MA_SUCCESS)
    {
        qWarning() << "EngineLaser: impossible d'initialiser l'audio";
        ma_context_uninit(&m_context);
        return 0;
    }

    ma_mutex_init(&m_mutex);

    // ── Charger le SoundFont ──
    m_tsf = tsf_load_filename(soundFontPath.toLocal8Bit().constData());

    if (!m_tsf) {
        qWarning() << "EngineLaser: SoundFont introuvable !";
        ma_device_uninit(&m_device);
        ma_mutex_uninit(&m_mutex);
        ma_context_uninit(&m_context);
        return 0;
    }

    if (ma_device_start(&m_device) != MA_SUCCESS) {
        qWarning() << "EngineLaser: impossible de démarrer la lecture";
        ma_device_uninit(&m_device);
        ma_mutex_uninit(&m_mutex);
        tsf_close(m_tsf);
        ma_context_uninit(&m_context);
        return 0;
    }

    m_audioOk = true;
    qDebug() << "EngineLaser: init sucessfull";
    return 1;
}

bool EngineLaser::isAudioOk()
{
    return m_audioOk;
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

    // mute all sound before changing instrument
    tsf_note_off_all(m_tsf);

    if (idInstrument < tsf_get_presetcount(m_tsf))
    {
        m_instrument = idInstrument;
        qDebug() << "EngineLaser : Instrument changé, nouvel instrument : " << idInstrument;
    }
    else
    {
        m_instrument = 0;
        qDebug() << "EngineLaser : Instrument non trouvé, fallback 0";
    }
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
void EngineLaser::jouerLaser(int laserId)
{
    if (!m_audioOk || !m_tsf) return;
    ma_mutex_lock(&m_mutex);
    for(int i = 0; i < m_accords[laserId].n_notes; i++)
    {
        tsf_note_on(m_tsf, m_instrument, m_accords[laserId].notes[i], m_volume);
    }
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::stopperLaser(int laserId)
{
    if (!m_audioOk || !m_tsf) return;
    ma_mutex_lock(&m_mutex);
    for(int i = 0; i < m_accords[laserId].n_notes; i++)
    {
        tsf_note_off(m_tsf, m_instrument, m_accords[laserId].notes[i]);
    }
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::jouerNoteDirecte(int noteMidi)
{
    if (!m_audioOk || !m_tsf) return;
    ma_mutex_lock(&m_mutex);
    tsf_note_on(m_tsf, m_instrument, noteMidi, m_volume);
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::stopperNoteDirecte(int noteMidi)
{
    if (!m_audioOk || !m_tsf) return;
    ma_mutex_lock(&m_mutex);
    tsf_note_off(m_tsf, m_instrument, noteMidi);
    ma_mutex_unlock(&m_mutex);
}

void EngineLaser::setVolume(float niveau)
{
    m_volume = niveau;
    if (m_tsf)
    {
        ma_mutex_lock(&m_mutex);
        tsf_set_volume(m_tsf, niveau);
        ma_mutex_unlock(&m_mutex);
        qDebug() << "EngineLaser : volume set to : " << niveau;
    }
}

// ─────────────────────────────────────────────
bool EngineLaser::initMidi(QString& midi_port_name)
{
    try {
        m_midiIn = new RtMidiIn();
        unsigned int nPorts = m_midiIn->getPortCount();
        if (nPorts == 0) {
            qWarning() << "EngineLaser : Aucun port MIDI disponible";
            delete m_midiIn;
            m_midiIn = nullptr;
            return false;
        }

        int id = 0; // fallback to port 0 if the requested port name isn't available (example : config file loading)
        for(unsigned int i = 0; i < nPorts; i++)
        {
            if(QString::fromStdString(m_midiIn->getPortName(i)) == midi_port_name)
            {
                id = i;
                break;
            }
        }

        m_midiIn->openPort(id);
        m_midiIn->setCallback(&EngineLaser::midiCallback, this);
        m_midiIn->ignoreTypes(false, false, false);
        qDebug() << "EngineLaser : MIDI ouvert:" << QString::fromStdString(m_midiIn->getPortName(id));
        return true;

    } catch (RtMidiError &e) {
        qWarning() << "EngineLaser : Erreur MIDI:" << QString::fromStdString(e.getMessage());
        return false;
    }
}

QStringList EngineLaser::getMidiPorts()
{
    QStringList liste;
    RtMidiIn* midiIn = new RtMidiIn();

    unsigned int nPorts = midiIn->getPortCount();

    if (nPorts == 0)
    {
        delete midiIn;
        midiIn = nullptr;
        qDebug() << "EngineLaser : no Midi device";
        return liste;
    }

    for (unsigned int i = 0; i < nPorts; i++)
        liste << QString::fromStdString(midiIn->getPortName(i));

    delete midiIn;
    midiIn = nullptr;

    qDebug() << "EngineLaser : returned Midi devices : " << liste;
    return liste;
}

QStringList EngineLaser::getAudioOuts()
{
    QStringList liste;

    ma_result result = ma_context_init(NULL, 0, NULL, &m_context);
    if (result != MA_SUCCESS)
    {
        return liste;
    }

    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;
    result = ma_context_get_devices(&m_context, &pPlaybackDeviceInfos, &playbackDeviceCount, NULL, NULL);
    if (result != MA_SUCCESS)
    {
        ma_context_uninit(&m_context);
        return liste;
    }

    for (ma_uint32  iDevice = 0; iDevice < playbackDeviceCount; ++iDevice)
        liste << pPlaybackDeviceInfos[iDevice].name;

    qDebug() << "EngineLaser : returned Audio Outputs : " << liste;

    return liste;
}

void EngineLaser::stopMidi()
{
    if (m_midiIn) {
        m_midiIn->cancelCallback();
        m_midiIn->closePort();
        delete m_midiIn;
        m_midiIn = nullptr;
        qDebug() << "EngineLaser : stopped Midi";
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
    int index_touche     = msg->at(1);
    int velocity = msg->at(2);

    if (status == 0x90 && velocity > 0)
    {
        ma_mutex_lock(&self->m_mutex);
        for(int i = 0; i < self->m_accords[index_touche].n_notes; i++)
        {
            tsf_note_on(self->m_tsf, self->m_instrument, self->m_accords[index_touche].notes[i], self->m_volume);
        }
        ma_mutex_unlock(&self->m_mutex);
        emit self->noteRecueMidi(index_touche, true);
    }
    else if (status == 0x80 || (status == 0x90 && velocity == 0))
    {
        ma_mutex_lock(&self->m_mutex);
        for(int i = 0; i < self->m_accords[index_touche].n_notes; i++)
        {
            tsf_note_off(self->m_tsf, self->m_instrument, self->m_accords[index_touche].notes[i]);
        }
        ma_mutex_unlock(&self->m_mutex);
        emit self->noteRecueMidi(index_touche, false);
    }
}

void EngineLaser::loadSoundFont(QString& fileName)
{
    tsf_close(m_tsf); // Free the memory used by the current SoundFont
    m_tsf = tsf_load_filename(fileName.toLocal8Bit().constData());
    if (!m_tsf)
    {
        qWarning() << "EngineLaser: SoundFont introuvable !";
        return;
    }

    qDebug() << "EngineLaser : loaded SoundFont : " << fileName;
    tsf_set_output(m_tsf, TSF_STEREO_INTERLEAVED, 44100, 0);
}

accord_t* EngineLaser::getAccords()
{
    accord_t* accordcpy = new accord_t[6];
    for(int i = 0; i < 6; i++)
    {
        accordcpy[i].n_notes = m_accords[i].n_notes;
        for(int j = 0; j < m_accords[i].n_notes; j++)
        {
            accordcpy[i].notes[j] = m_accords[i].notes[j];
        }
    }
    qDebug() << "EngineLaser : get accords";
    return accordcpy;
}

void EngineLaser::setAccords(accord_t* accordcpy)
{
    if(m_accords != nullptr)
    {
        delete m_accords;
        m_accords = nullptr;
    }

    m_accords = new accord_t[6];
    for(int i = 0; i < 6; i++)
    {
        m_accords[i].n_notes = accordcpy[i].n_notes;
        for(int j = 0; j < accordcpy[i].n_notes; j++)
        {
            m_accords[i].notes[j] = accordcpy[i].notes[j];
        }
    }
    qDebug() << "EngineLaser : accords set";
}
