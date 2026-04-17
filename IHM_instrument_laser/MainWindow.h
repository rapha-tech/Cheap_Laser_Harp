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
#include "EngineLaser.h"
#include "configFile.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void isEngineOk();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void fermer();
    void allumerBarre(int id);
    void jouerLaser(int id);
    void stopperLaser(int id);
    void eteindreBarre(int id);
    void activerAssignation(int id);
    void assignerNoteLaser(int noteMidi);
    void nouveau();
    void actif();
    void loadSF2();
    void connectMidi(int id);
    void updatePorts();
    void updateListInstruments(QString&);
    void loadInstrument(int id);
    void loadConfig();
    void saveConfig();
    void saveConfigAs();

private:
    void repositionnerTouchesNoires();
    void surlignerToucheAssignee(int laserId);
    void resetStylePiano();

    QString configPath;

    QMenu *mListePeripheriques;
    QSlider *sliderVol;

    QVector<QFrame*> m_barres;
    QVector<QPushButton*> m_btnLaser;
    QVector<QPushButton*> m_btnAssign;

    QWidget* m_pianoWidget;
    QVector<QPushButton*> m_touchesBlanches;
    QVector<QPushButton*> m_touchesNoires;

    QMap<int, int> m_laserNote;
    QMap<int, bool> m_laserNoteEstNoire;
    int m_laserEnAssignation;

    QListWidget* m_choixInstrument;
    QVector<QLabel*> m_labelsNotes;

    EngineLaser* m_engine;
    configFile* m_configFile;

    static const QStringList NOTES_NAMES;
    static const QList<int> IS_NOIRE;
};

#endif
