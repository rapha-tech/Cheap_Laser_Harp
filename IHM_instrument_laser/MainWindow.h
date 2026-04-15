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

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void fermer();
    void allumerBarre(int id);
    void jouerLaser(int id);
    void stopperLaser(int id);
    void eteindreBarre(int id);
    void activerAssignation(int id);
    void assignerNoteLaser(int noteIndex, bool estNoire);
    void nouveau();
    void actif();
    void loadSF2();
    void connectMidi(int id);
    void updatePorts();

private:
    void repositionnerTouchesNoires();
    void surlignerToucheAssignee(int laserId);
    void resetStylePiano();

    QMenu *mListePeripheriques;

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

    static const QStringList NOTES_BLANCHES;
    static const QStringList NOTES_NOIRES;
    static const QList<int> POSITIONS_NOIRES;
};

#endif
