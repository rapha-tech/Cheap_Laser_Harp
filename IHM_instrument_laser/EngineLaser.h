#ifndef ENGINELASER_H
#define ENGINELASER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

#include "lib/miniaudio.h"
#include "lib/tsf.h"
#include "lib/RtMidi.h"

struct accord_t
{
    int n_notes;
    int notes[24];
};

class EngineLaser : public QObject {
    Q_OBJECT
public:
    explicit EngineLaser(QObject *parent);
    ~EngineLaser();
    bool initEngine(QString& soundFontPath);
    bool isAudioOk();

    // Instrument
    void         loadSoundFont(QString&);
    QStringList  getInstrumentsDisponibles() const;
    void         chargerInstrument(int idInstrument);


    // Notes lasers
    void jouerLaser(int laserId);
    void stopperLaser(int laserId);

    // Notes directes depuis le piano
    void jouerNoteDirecte(int noteMidi);
    void stopperNoteDirecte(int noteMidi);

    // Volume
    void setVolume(float niveau);

    // MIDI
    QStringList getMidiPorts();
    bool initMidi(unsigned int id = 0);
    void stopMidi();

    // Utilitaire
    accord_t* getAccords();
    void setAccords(accord_t* accordcpy);

signals:
    void noteRecueMidi(int note, bool active);

private:

    static void audioCallback(ma_device* dev, void* out,
                              const void* in, ma_uint32 frames);
    static void midiCallback(double dt,
                             std::vector<unsigned char>* msg,
                             void* userData);

    tsf*            m_tsf       = nullptr;
    ma_device       m_device;
    ma_mutex        m_mutex;
    bool            m_audioOk   = false;

    accord_t*       m_accords;
    float           m_volume    = 0.8f;
    int             m_instrument= 0;

    RtMidiIn*       m_midiIn    = nullptr;
};

#endif
