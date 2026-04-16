#ifndef ENGINELASER_H
#define ENGINELASER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

#include "lib/miniaudio.h"
#include "lib/tsf.h"
#include "lib/RtMidi.h"

class EngineLaser : public QObject {
    Q_OBJECT
public:
    explicit EngineLaser(QObject *parent, QString&);
    ~EngineLaser();

    // Instrument
    void         chargerInstrument(int idInstrument);
    QStringList  getInstrumentsDisponibles() const;
    void loadSoundFont(QString&);

    // Notes lasers
    void setNoteIndex(int laserId, int noteIndex, bool estNoire);
    void jouerNote(int laserId);
    void stopperNote(int laserId);

    // Notes directes depuis le piano
    void jouerNoteDirecte(int noteIndex, bool estNoire);
    void stopperNoteDirecte(int noteIndex, bool estNoire);

    // Volume
    void setVolume(float niveau);

    // MIDI
    bool initMidi(unsigned int id = 0);
    void stopMidi();
    QStringList getMidiPorts();

    // Utilitaire
    int  midiNoteForLaser(int laserId);

    // Compatibilité ancienne interface
    void configurerNote(int laserId, const QString &chemin) { Q_UNUSED(laserId) Q_UNUSED(chemin) }

signals:
    void noteRecueMidi(int note, bool active);

private:
    int  noteIndexToMidi(int noteIndex, bool estNoire);

    static void audioCallback(ma_device* dev, void* out,
                              const void* in, ma_uint32 frames);
    static void midiCallback(double dt,
                             std::vector<unsigned char>* msg,
                             void* userData);

    tsf*            m_tsf       = nullptr;
    ma_device       m_device;
    ma_mutex        m_mutex;
    bool            m_audioOk   = false;

    QMap<int, int>  m_laserMidiNote;
    float           m_volume    = 0.8f;
    int             m_instrument= 0;

    RtMidiIn*       m_midiIn    = nullptr;

    static const int MIDI_BASE = 60;
};

#endif
