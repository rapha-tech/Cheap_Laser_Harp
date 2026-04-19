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

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void isEngineOk();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void fermer();
    void jouerLaser(int id);
    void stopperLaser(int id);
    void allumerBarre(int id);
    void eteindreBarre(int id);
    void toggleAssignation(int id);
    void toggleTouche(int noteMidi);
    void nouveau();
    void actif();
    void getSoundFontPath();
    void loadSoundFont(QString& soundFontPath);
    void setVolume(int val);
    void updateLatestSoundFonts();
    void connectMidi(int id);
    void updatePorts();
    void loadInstrument(int id);
    void getConfigPathLoad();
    void loadConfig(QString& configPath);
    void updateLatestConfigs();
    void saveConfig();
    void saveConfigAs();

private:
    void repositionnerTouchesNoires();
    void resetStylePiano();

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    QString m_configPath;
    QString m_soundFontPath;
    int m_instrumentId = -1;
    int m_volume = -1;

    QMenu *mListePeripheriques;
    QMenu *mListeLatestConfigs;
    QMenu *mListeLatestSoundFont;
    QSlider *sliderVol;

    QVector<QFrame*> m_barres;
    QVector<QPushButton*> m_btnLaser;
    QVector<QPushButton*> m_btnAssign;

    QWidget* m_pianoWidget;
    QVector<QPushButton*> m_touchesBlanches;
    QVector<QPushButton*> m_touchesNoires;

    accord_t* m_accords;
    int m_laserEnAssignation;
    int m_pselectedTouches[24];

    QListWidget* m_choixInstrument;
    QVector<QLabel*> m_labelsNotes;

    EngineLaser* m_engine;
    configFile* m_configFile;
    RecentFiles* m_recentFiles;

    static const QStringList NOTES_NAMES;
    static const QList<int> IS_NOIRE;

    QMap<int, int> NOTE_MIDI_TO_ID_TOUCHE_NOIRE;
    QMap<int, int> NOTE_MIDI_TO_ID_TOUCHE_BLANCHE;
};

#endif
