#include "MainWindow.h"
#include "EngineLaser.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QMessageBox>
#include <QDebug>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>



const QStringList MainWindow::NOTES_BLANCHES = {
    "Do","Re","Mi","Fa","Sol","La","Si",
    "Do","Re","Mi","Fa","Sol"
};
const QStringList MainWindow::NOTES_NOIRES = {
    "Do#","Re#","","Fa#","Sol#","La#","",
    "Do#","Re#","","Fa#",""
};
const QList<int> MainWindow::POSITIONS_NOIRES = {0,1,3,4,5,7,8,10};

static const QString S_BLANCHE =
    "QPushButton {"
    "  background-color: #FFFFFF;"
    "  border: 1px solid #000000;"
    "  color: black;"
    "  font-size: 9px;"
    "}"
    "QPushButton:hover {"
    "  background-color: #D0D0D0;"
    "}"
    "QPushButton:pressed {"
    "  background-color: #808080;"
    "}";
static const QString S_BLANCHE_SEL =
    "QPushButton {"
     " background-color: rgba(30, 60, 120, 180);"  // bleu nuit
     " border: 2px solid rgba(100, 150, 255, 220);" // contour lumineux
    "}";

static const QString S_NOIRE =
    "QPushButton {"
    "  background-color: #111;"
    "  border-radius: 0 0 4px 4px;"
    "  border-left:   1px solid #333;"
    "  border-right:  1px solid #000;"
    "  border-bottom: 1px solid #000;"
    "  border-top: none;"
    "}"
                               "QPushButton:hover {"
                               "  background-color: #333333;"
                               "}"
                               "QPushButton:pressed {"
                               "  background-color: #000000;"
                               "  border: 2px solid #888888;"
                               "}";

static const QString S_NOIRE_SEL =
    "QPushButton {"
    "background-color: rgba(60, 90, 150, 255);"
    " border: 2px solid rgba(100, 150, 255, 220);" // contour lumineux
    "}";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_laserEnAssignation(-1)
{
    //this->setStyleSheet("QWidget { background-image: url(:/1.png); }");
    //this->setStyleSheet("MainWindow { background-image: url(:/1.png); }");
    m_engine = new EngineLaser(this);

    for (int i = 1; i <= 6; i++) {
        m_laserNote[i]         = i - 1;
        m_laserNoteEstNoire[i] = false;
    }

    QMenu *mFichier = menuBar()->addMenu("&Fichier");
    QAction *actionnv = new QAction("&Nouveau projet ", this);
    mFichier->addAction(actionnv);
    actionnv->setShortcut(QKeySequence(tr("Ctrl + N")));
    connect(actionnv, SIGNAL(triggered()), this, SLOT(nouveau()));


    mFichier->addAction("&Ouvrir...");
    mFichier->addAction("&Enregistrer");
    mFichier->addSeparator();

    QAction *actionQuitter = new QAction("&Quitter", this);
    mFichier->addAction(actionQuitter);
    actionQuitter->setShortcut(QKeySequence(tr("Ctrl + Q")));
    connect(actionQuitter, SIGNAL(triggered()), this, SLOT(close()));



    QMenu *mInstrument = new QMenu("&Instrument");

    QAction *actionSF2 = new QAction("&Importer SF2...", this);
    mInstrument->addAction(actionSF2);
    connect(actionnv, SIGNAL(triggered()), this, SLOT(loadSF2()));


    mListePeripheriques = new QMenu("&Peripheriques");

    updatePorts();

    mInstrument->addMenu(mListePeripheriques);

    // this action is used to update the list of available Midi ports
    QAction *actionInstr = menuBar()->addMenu(mInstrument);
    connect(actionInstr, &QAction::hovered, this, [=]() { // FIXME : use triggered instead of hovered
        updatePorts();
    });



    QMenu *mLasers = menuBar()->addMenu("&Lasers");
    QAction *reinitialiser = new QAction("&Reinitialiser les assignations", this);
    mLasers->addAction(reinitialiser);
    reinitialiser->setShortcut(QKeySequence(tr("Ctrl + R")));

    connect(reinitialiser,
            &QAction::triggered, this, [=]() {
                for (int i = 1; i <= 6; i++) {
                    m_laserNote[i] = i - 1;
                    m_laserNoteEstNoire[i] = false;
                    m_engine->setNoteIndex(i, i - 1, false);
                    m_btnAssign[i-1]->setText(NOTES_BLANCHES[i-1]);
                    m_labelsNotes[i-1]->setText(NOTES_BLANCHES[i-1]);
                }
            });
    mLasers->addAction("&Parametres...");

    QMenu *mAffichage = menuBar()->addMenu("&Affichage");
    QAction *actionFullScreen = mAffichage->addAction("&Passer en plein écran");

    connect(actionFullScreen, &QAction::triggered, this, [=]() {
        if (isFullScreen()) {
            showNormal();
            actionFullScreen->setText("Passer en plein écran");
        } else {
            showFullScreen();
            actionFullScreen->setText("Quitter le plein écran");
        }
    });

    QMenu *mAide = menuBar()->addMenu("Aide");

    // ── Layout principal ──
    QWidget *central = new QWidget(this);
    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(10);

    QWidget *leftPanel = new QWidget();
    //leftPanel->setFixedWidth(160);
    QVBoxLayout *left = new QVBoxLayout(leftPanel);
    left->setContentsMargins(0, 0, 0, 0);
    left->setSpacing(6);

    QLabel *titre = new QLabel("Configuration");
    titre->setStyleSheet("font-weight: bold; color: #00BFFF;");
    left->addWidget(titre);

    m_choixInstrument = new QListWidget;
    left->addWidget(m_choixInstrument);

    left->addWidget(new QLabel("Volume"));
    QSlider *sliderVol = new QSlider(Qt::Horizontal);
    sliderVol->setRange(0, 100);
    sliderVol->setValue(80);
    left->addWidget(sliderVol);

    left->addSpacing(8);
    QLabel *titleNotes = new QLabel("Notes assignées");
    titleNotes->setStyleSheet("font-weight: bold; color: #00BFFF;");
    left->addWidget(titleNotes);

    QWidget *noteListWidget = new QWidget();
    QGridLayout *noteGrid = new QGridLayout(noteListWidget);
    noteGrid->setContentsMargins(0, 2, 0, 0);
    noteGrid->setVerticalSpacing(3);
    noteGrid->setHorizontalSpacing(6);
    m_labelsNotes.resize(6);

    for (int i = 0; i < 6; i++) {
        QLabel *laserLbl = new QLabel(QString("Laser %1 :").arg(i + 1));
        laserLbl->setStyleSheet("font-size: 11px;");
        QLabel *noteLbl  = new QLabel(NOTES_BLANCHES[i]);
        noteLbl->setStyleSheet(
            "font-size: 11px; font-weight: bold; color: #ff66cc;");
        m_labelsNotes[i] = noteLbl;
        noteGrid->addWidget(laserLbl, i, 0);
        noteGrid->addWidget(noteLbl,  i, 1);
    }
    left->addWidget(noteListWidget);
    left->addStretch();



    QPushButton *rendreactif = new QPushButton("Rendre Actif");
    rendreactif->setShortcut(QKeySequence(tr("Ctrl + R")));

    connect(rendreactif, &QPushButton::clicked, this, &MainWindow::actif);
    left->addWidget(rendreactif);


    QPushButton *quitBtn = new QPushButton("Quitter");
    quitBtn->setShortcut(QKeySequence(tr("Ctrl + W")));

    connect(quitBtn, &QPushButton::clicked, this, &MainWindow::fermer);
    left->addWidget(quitBtn);

    //  ZONE CENTRALE
    // ══════════════════════════════════════════
    QWidget *center = new QWidget();
    center->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *centerLayout = new QVBoxLayout(center);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(8);

    // Lasers
    QWidget *laserZone = new QWidget();
    QHBoxLayout *laserRow = new QHBoxLayout(laserZone);
    laserRow->setContentsMargins(0, 0, 0, 0);
    laserRow->setSpacing(0);

    m_barres.resize(6);
    m_btnLaser.resize(6);
    m_btnAssign.resize(6);

    for (int i = 1; i <= 6; i++) {
        QWidget *col = new QWidget();
        QVBoxLayout *colL = new QVBoxLayout(col);
        colL->setContentsMargins(3, 0, 3, 0);
        colL->setSpacing(4);
        colL->setAlignment(Qt::AlignHCenter);

        QFrame *barre = new QFrame();
        barre->setMinimumHeight(100);
        barre->setFixedWidth(12);
        barre->setStyleSheet("background-color: #222; border-radius: 5px;");
        m_barres[i - 1] = barre;

        QPushButton *btn = new QPushButton(QString("Laser %1").arg(i));

        btn->setMinimumHeight(100);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet(
            "QPushButton { background-color: #333; border-radius: 8px; color: #aaa; }"
            "QPushButton:hover { background-color: #444;}"
            "QPushButton:pressed { background-color: #ff00ff;}"
            );
        connect(btn, &QPushButton::pressed,  this, [=]() {
            jouerLaser(i);
            allumerBarre(i);
            btn->setIcon(QIcon(":/allume.png"));

        });
        connect(btn, &QPushButton::released, this, [=]() {
            stopperLaser(i);
            eteindreBarre(i);
        });
        m_btnLaser[i - 1] = btn;

        QPushButton *assignBtn = new QPushButton(NOTES_BLANCHES[i - 1]);
        assignBtn->setMinimumHeight(28);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        assignBtn->setStyleSheet(
            "QPushButton { background-color: #222; border-radius: 4px;"
            "  border: 1px solid #444; color: #aaa; font-size: 11px; }"
            "QPushButton:hover { border-color: #ff00ff; color: #ff00ff; }"
            );
        connect(assignBtn, &QPushButton::clicked, this, [=]() {
            activerAssignation(i);
        });
        m_btnAssign[i - 1] = assignBtn;

        colL->addWidget(barre, 50, Qt::AlignHCenter);
        colL->addWidget(btn, 30);
        colL->addWidget(assignBtn, 20);
        laserRow->addWidget(col);
    }

    // Piano
    QWidget *pianoZone = new QWidget();
    QVBoxLayout *pianoVBox = new QVBoxLayout(pianoZone);
    pianoVBox->setContentsMargins(0, 0, 0, 0);
    pianoVBox->setSpacing(4);

    QLabel *pianoHint = new QLabel(
        "Activer un bouton d'assignation puis cliquer une touche - Les touches sont cliquables"
        );
    pianoHint->setStyleSheet("font-size: 10px; color: #888;");

    m_pianoWidget = new QWidget();
    m_pianoWidget->setMinimumHeight(120);
    m_pianoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *pianoLay = new QHBoxLayout(m_pianoWidget);
    pianoLay->setSpacing(1);
    pianoLay->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < NOTES_BLANCHES.size(); i++) {
        QPushButton *blanc = new QPushButton(NOTES_BLANCHES[i]);
        blanc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        blanc->setMinimumWidth(10);
        blanc->setStyleSheet(S_BLANCHE);
        connect(blanc, &QPushButton::pressed, this, [=]() {
            m_engine->jouerNoteDirecte(i, false);
        });
        connect(blanc, &QPushButton::released, this, [=]() {
            m_engine->stopperNoteDirecte(i, false);
            if (m_laserEnAssignation >= 0)
                assignerNoteLaser(i, false);
        });
        m_touchesBlanches.append(blanc);
        pianoLay->addWidget(blanc);
    }

    // Overlay touches noires
    QWidget *blackOverlay = new QWidget(m_pianoWidget);
    blackOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    blackOverlay->setGeometry(0, 0, m_pianoWidget->width(), 75);
    blackOverlay->raise();

    for (int n = 0; n < POSITIONS_NOIRES.size(); n++) {
        int idx = POSITIONS_NOIRES[n];
        if (idx >= NOTES_NOIRES.size() || NOTES_NOIRES[idx].isEmpty()) continue;

        QPushButton *noir = new QPushButton();
        noir->setFixedSize(18, 75);
        noir->setStyleSheet(S_NOIRE);
        noir->setParent(blackOverlay);
        noir->raise();
        m_touchesNoires.append(noir);

        connect(noir, &QPushButton::pressed, this, [=]() {
            m_engine->jouerNoteDirecte(idx, true);
        });
        connect(noir, &QPushButton::released, this, [=]() {
            m_engine->stopperNoteDirecte(idx, true);
            if (m_laserEnAssignation >= 0)
                assignerNoteLaser(idx, true);
        });
    }

    pianoVBox->addWidget(pianoHint);
    pianoVBox->addWidget(m_pianoWidget);

    centerLayout->addWidget(laserZone);
    centerLayout->addWidget(pianoZone);

    root->addWidget(leftPanel, 20);
    root->addWidget(center, 80);

    // ── Connexions ──
    connect(m_choixInstrument, &QListWidget ::currentRowChanged,
            m_engine, &EngineLaser::chargerInstrument);

    connect(sliderVol, &QSlider::valueChanged, this, [=](int val) {
        m_engine->setVolume(val / 100.0f);
    });

    connect(m_engine, &EngineLaser::noteRecueMidi,
            this, [=](int midiNote, bool active) {
                for (int i = 1; i <= 6; i++) {
                    if (m_engine->midiNoteForLaser(i) == midiNote) {
                        if (active) allumerBarre(i);
                        else        eteindreBarre(i);
                    }
                }
            });

    // Remplir combobox avec instruments SF2
    QStringList instruments = m_engine->getInstrumentsDisponibles();
    if (!instruments.isEmpty()) {
        m_choixInstrument->blockSignals(true);
        m_choixInstrument->addItems(instruments);
        m_choixInstrument->blockSignals(false);
    } else {
        m_choixInstrument->addItems({"0 - Piano"});
    }

    m_engine->chargerInstrument(m_choixInstrument->currentRow());
    m_engine->setVolume(0.8f);
    m_engine->initMidi();

    qDebug() << "=== Instruments SF2 ===";
    for (const QString &s : m_engine->getInstrumentsDisponibles())
        qDebug() << s;

    setCentralWidget(central);
    setWindowTitle("Instrument Laser");
    setMinimumSize(700, 480);

    QTimer::singleShot(0, this, [this]() {
        repositionnerTouchesNoires();
    });
    connect(mAide->addAction("&A propos"), &QAction::triggered, this, [=]() {
        QMessageBox about(this);
        about.setWindowTitle("A propos");
        about.setIcon(QMessageBox::Information);
        about.setText(
            "<h3>Harpe Laser</h3>"
            "<p>Projet réalisé dans le cadre d'un projet de 3ème année.</p>"
            "<p><b>Equipe :</b> Lot 2</p>"
            "<hr>"
            "<p>Un instrument de musique laser interactif — "
            "assignez des notes à chaque laser, "
            "choisissez votre instrument et jouez !</p>"
            "<p><i>Amusez-vous bien !</i></p>"

            );
        about.setStandardButtons(QMessageBox::Ok);
        about.exec();
    });
}

// ─────────────────────────────────────────────
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    repositionnerTouchesNoires();
}



void MainWindow::repositionnerTouchesNoires() {
    if (m_touchesBlanches.isEmpty() || m_touchesNoires.isEmpty()) return;
    if (!m_pianoWidget) return;

    QWidget *overlay = m_touchesNoires.first()->parentWidget();
    if (!overlay) return;
    overlay->setGeometry(0, 0, m_pianoWidget->width(), 75);

    int noirIdx = 0;
    for (int idx : POSITIONS_NOIRES) {
        if (noirIdx >= m_touchesNoires.size()) break;
        if (idx + 1 >= m_touchesBlanches.size()) { noirIdx++; continue; }

        QRect gL = m_touchesBlanches[idx]->geometry();
        QRect gR = m_touchesBlanches[idx + 1]->geometry();
        int xCenter = (gL.right() + gR.left()) / 2;
        m_touchesNoires[noirIdx]->move(xCenter - 9, 0);
        m_touchesNoires[noirIdx]->raise();
        noirIdx++;
    }
}

void MainWindow::resetStylePiano() {
    for (auto t : m_touchesBlanches) t->setStyleSheet(S_BLANCHE);
    for (auto t : m_touchesNoires)   t->setStyleSheet(S_NOIRE);
}

void MainWindow::surlignerToucheAssignee(int laserId) {
    resetStylePiano();
    int  noteIdx  = m_laserNote.value(laserId, -1);
    bool estNoire = m_laserNoteEstNoire.value(laserId, false);

    if (estNoire) {
        int noirIdx = POSITIONS_NOIRES.indexOf(noteIdx);
        if (noirIdx >= 0 && noirIdx < m_touchesNoires.size())
            m_touchesNoires[noirIdx]->setStyleSheet(S_NOIRE_SEL);
    } else {
        if (noteIdx >= 0 && noteIdx < m_touchesBlanches.size())
            m_touchesBlanches[noteIdx]->setStyleSheet(S_BLANCHE_SEL);
    }
}

void MainWindow::activerAssignation(int laserId) {
    m_laserEnAssignation = laserId;

    for (int i = 0; i < 6; i++) {
        bool actif = (i + 1 == laserId);
        QString note = m_laserNoteEstNoire[i+1]
                           ? NOTES_NOIRES[m_laserNote[i+1]]
                           : NOTES_BLANCHES[m_laserNote[i+1]];
        m_btnAssign[i]->setText(actif ? "..." : note);
        m_btnAssign[i]->setStyleSheet(
            actif
                ? "QPushButton { background-color: #330033; border-radius: 4px;"
                  "  border: 1px solid #ff00ff; color: #ff00ff; font-size: 11px; }"
                : "QPushButton { background-color: #222; border-radius: 4px;"
                  "  border: 1px solid #444; color: #aaa; font-size: 11px; }"
                  "QPushButton:hover { border-color: #ff00ff; color: #ff00ff; }"
            );
    }
    surlignerToucheAssignee(laserId);
}

void MainWindow::assignerNoteLaser(int noteIndex, bool estNoire) {
    if (m_laserEnAssignation < 0) return;

    int id = m_laserEnAssignation;
    m_laserNote[id]         = noteIndex;
    m_laserNoteEstNoire[id] = estNoire;

    QString nomNote = estNoire
                          ? NOTES_NOIRES[noteIndex]
                          : NOTES_BLANCHES[noteIndex];

    m_labelsNotes[id - 1]->setText(nomNote);
    m_btnAssign[id - 1]->setText(nomNote);
    m_btnAssign[id - 1]->setStyleSheet(
        "QPushButton { background-color: #222; border-radius: 4px;"
        "  border: 1px solid #444; color: #aaa; font-size: 11px; }"
        "QPushButton:hover { border-color: #ff00ff; color: #ff00ff; }"
        );

    m_engine->setNoteIndex(id, noteIndex, estNoire);

    resetStylePiano();
    m_laserEnAssignation = -1;
}

void MainWindow::jouerLaser(int id) {
    m_engine->jouerNote(id);
}

void MainWindow::stopperLaser(int id) {
    m_engine->stopperNote(id);
}

// ─────────────────────────────────────────────
void MainWindow::allumerBarre(int id) {
    m_barres[id-1]->setStyleSheet(
        "background-color: #ff00ff; border-radius: 5px;");
}

void MainWindow::eteindreBarre(int id) {
    m_barres[id-1]->setStyleSheet(
        "background-color: #222; border-radius: 5px;");
}

// ─────────────────────────────────────────────
void MainWindow::fermer() {
    if (QMessageBox::warning(this, "Quitter", "Fermer l'application ?", QMessageBox:: Yes | QMessageBox:: No)
        == QMessageBox::Yes)
        close();
}

void MainWindow::nouveau() {
    MainWindow *newWindow = new MainWindow();
    newWindow->show();

    //this->close();
}

void MainWindow::actif() {
    this->show();
}

void MainWindow::loadSF2()
{

}

void MainWindow::connectMidi(int id)
{
    m_engine->stopMidi();
    m_engine->initMidi(id);
}

void MainWindow::updatePorts()
{
    if(!mListePeripheriques->isEmpty())
    {
        mListePeripheriques->clear();
    }

    // On ajouter les périphériques disponibles (connectés au PC)
    QStringList ListePorts = m_engine->getMidiPorts();

    if(ListePorts.size() > 0)
    {

        for(unsigned int i = 0; i < ListePorts.size(); i++)
        {
            QAction* actionsPort = new QAction(ListePorts[i], this);
            mListePeripheriques->addAction(actionsPort);
            connect(actionsPort, &QAction::triggered, this, [=]() {
                connectMidi(i);
            });
        }
    }
    else
    {
        mListePeripheriques->addAction("&Aucun peripherique compatible");
    }
}
