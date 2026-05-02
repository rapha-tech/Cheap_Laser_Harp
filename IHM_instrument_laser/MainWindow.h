#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QFrame>
#include <QMap>
#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QListWidget>
#include <QKeyEvent>

#include "EngineLaser.h"
#include "configFile.h"
#include "RecentFiles.h"
#include "qscrollarea.h"

#define NOTES_OCTAVE 12
#define NOTES_DISPLAYED 24
#define DEFAULT_OCTAVE 2 * NOTES_OCTAVE
#define NOTES_TOTAL 7 * NOTES_OCTAVE

#define DEFAULT_CONFIG_PATH "config.json"

#define SAFE_DELETE(pointer) \
if(pointer != nullptr)       \
{                            \
    delete pointer;          \
    pointer = nullptr;       \
}                            \


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void isEngineOk();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void fermer();
    void jouerLaser(int id);
    void stopperLaser(int id);
    void allumerBarre(int id);
    void eteindreBarre(int id);
    void toggleAssignation(int id);
    void toggleTouche(int noteMidi);
    void toggleNotes();
    void nouveau();
    void actif();
    void getSoundFontPath();
    void loadSoundFont(QString& soundFontPath, int instrumentId = 0);
    void setVolume(int val, bool slider = true);
    void updateLatestSoundFonts();
    void connectMidi(int id);
    void updatePorts();
    void updateAudioOuts();
    void loadInstrument(int id, bool setRow = true);
    void getConfigPathLoad();
    void loadConfig(QString& configPath);
    void updateLatestConfigs();
    void saveConfig();
    void saveConfigAs();
    void resizeEvent(QResizeEvent *event);

private:
    void resetStylePiano();

    bool m_darkMode = true;
    void applyTheme();
    QWidget     *leftPanel;
    QScrollArea *pianoScroll;
    QWidget     *pianoCanvas;


    QString m_configPath;
    QString m_soundFontPath;
    int m_instrumentId = -1;
    int m_volume = -1;

    QMenu *mListePeripheriques;
    QMenu *mListeLatestConfigs;
    QMenu *mListeLatestSoundFont;
    QMenu *mListeAudioOut;
    QSlider *sliderVol;

    QStringList m_listePortsMidi;

    QVector<QFrame*> m_barres;
    QVector<QPushButton*> m_btnLaser;
    QVector<QPushButton*> m_btnAssign;

    QWidget* m_pianoWidget;
    QVector<QPushButton*> m_touchesBlanches;
    QVector<QPushButton*> m_touchesNoires;

    accord_t* m_accords = nullptr;
    int m_laserEnAssignation;
    int m_pselectedTouches[NOTES_TOTAL];

    QListWidget* m_choixInstrument;
    QVector<QLabel*> m_labelsNotes;

    EngineLaser* m_engine;
    configFile* m_configFile = nullptr;
    RecentFiles* m_recentFiles;

    static const QStringList NOTES_NAMES;
    static const QList<int> IS_NOIRE;

    QMap<int, int> NOTE_MIDI_TO_ID_TOUCHE_NOIRE;
    QMap<int, int> NOTE_MIDI_TO_ID_TOUCHE_BLANCHE;
};

#endif
