#include "MainWindow.h"
#include "EngineLaser.h"
#include "configFile.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QDebug>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>

#define NOTES_DISPLAYED 24
#define DEFAULT_OCTAVE 4 * 12

const QStringList MainWindow::NOTES_NAMES = {
    "Do","Do#","Re","Re#","Mi","Fa","Fa#","Sol","Sol#","La","La#","Si"
};
const QList<int> MainWindow::IS_NOIRE = {0,1,0,1,0,0,1,0,1,0,1,0};

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

static const QString S_BLANCHE_PLAY =
    "QPushButton {"
    " background-color: #ff00ff;" // violet
    " border: 1px solid #000000;" // contour noir
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

static const QString S_NOIRE_PLAY =
    "QPushButton {"
    "background-color: #ff00ff;"  // violet
    " border: 1px solid #000000;" // contour noir
    "}";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_laserEnAssignation(-1)
{
    //this->setStyleSheet("QWidget { background-image: url(:/1.png); }");
    //this->setStyleSheet("MainWindow { background-image: url(:/1.png); }");


    // we precompute the mapping on startup to save time during real time operation
    int noteIdNoire;
    int noteIdBlanche;
    for(int i = 0; i < NOTES_DISPLAYED; i++)
    {
        noteIdNoire = 0;
        noteIdBlanche = 0;
        for(int j = 0; j < i; j++)
        {
            if(IS_NOIRE[j % 12])
            {
                noteIdNoire++;
            }
            else
            {
                noteIdBlanche++;
            }
        }
        NOTE_MIDI_TO_ID_TOUCHE_NOIRE[i] = noteIdNoire;
        NOTE_MIDI_TO_ID_TOUCHE_BLANCHE[i] = noteIdBlanche;
    }

    // load config file
    m_configPath = QString("config.json");
    m_configFile = new configFile(m_configPath);

    // get settings
    QString soundFontPath = m_configFile->get_soundFont_path();
    int instrumentId = m_configFile->get_instr_id();
    int volume = m_configFile->get_volume();
    int port_id = m_configFile->get_port_id();
    accord_t* accords = m_configFile->getAccords();

    m_engine = new EngineLaser(this);
    m_engine->initEngine(soundFontPath);

    QMenu *mFichier = menuBar()->addMenu("&Fichier");
    QAction *actionnv = new QAction("&Nouveau projet ", this);
    mFichier->addAction(actionnv);
    actionnv->setShortcut(QKeySequence(tr("Ctrl + N")));
    connect(actionnv, SIGNAL(triggered()), this, SLOT(nouveau()));

    QAction *actionCharger = new QAction("&Charger configuration", this);
    mFichier->addAction(actionCharger);
    connect(actionCharger, SIGNAL(triggered()), this, SLOT(loadConfig()));

    QAction *actionSave = new QAction("&Enregistrer", this);
    mFichier->addAction(actionSave);
    actionSave->setShortcut(QKeySequence(tr("Ctrl + S")));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(saveConfig()));

    QAction *actionSaveAs = new QAction("&Enregistrer sous", this);
    mFichier->addAction(actionSaveAs);
    connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(saveConfigAs()));

    mFichier->addSeparator();

    QAction *actionQuitter = new QAction("&Quitter", this);
    mFichier->addAction(actionQuitter);
    actionQuitter->setShortcut(QKeySequence(tr("Ctrl + Q")));
    connect(actionQuitter, SIGNAL(triggered()), this, SLOT(close()));



    QMenu *mInstrument = new QMenu("&Instrument");

    QAction *actionSF2 = new QAction("&Charger SoundFont", this);
    mInstrument->addAction(actionSF2);
    connect(actionSF2, SIGNAL(triggered()), this, SLOT(loadSF2()));


    mListePeripheriques = new QMenu("&Peripheriques");

    updatePorts();

    mInstrument->addMenu(mListePeripheriques);

    // this action is used to update the list of available Midi ports when the menu bar is triggered
    menuBar()->addMenu(mInstrument);
    connect(mInstrument, &QMenu::aboutToShow, this, [=]() {
        updatePorts();
    });


    QMenu *mLasers = menuBar()->addMenu("&Lasers");
    QAction *reinitialiser = new QAction("&Reinitialiser les assignations", this);
    mLasers->addAction(reinitialiser);
    reinitialiser->setShortcut(QKeySequence(tr("Ctrl + R")));

    connect(reinitialiser,
            &QAction::triggered, this, [=]() {
                for (int i = 0; i < 6; i++) {
                    m_accords = m_configFile->get_default_accord();
                    m_engine->setAccords(m_accords);
                    delete m_accords;
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
    leftPanel->setMinimumWidth(160);
    QVBoxLayout *left = new QVBoxLayout(leftPanel);
    left->setContentsMargins(0, 0, 0, 0);
    left->setSpacing(6);

    QLabel *titre = new QLabel("Configuration");
    titre->setStyleSheet("font-weight: bold; color: #00BFFF;");
    left->addWidget(titre);

    m_choixInstrument = new QListWidget;
    left->addWidget(m_choixInstrument, 70); // this is to "fill" the left panel with Qlist elements

    left->addWidget(new QLabel("Volume"));
    sliderVol = new QSlider(Qt::Horizontal);
    sliderVol->setRange(0, 100);
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
        QLabel *noteLbl  = new QLabel(NOTES_NAMES[i]);
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

    for (int i = 0; i < 6; i++) {
        QWidget *col = new QWidget();
        QVBoxLayout *colL = new QVBoxLayout(col);
        colL->setContentsMargins(3, 0, 3, 0);
        colL->setSpacing(4);
        colL->setAlignment(Qt::AlignHCenter);

        QFrame *barre = new QFrame();
        barre->setMinimumHeight(100);
        barre->setFixedWidth(12);
        barre->setStyleSheet("background-color: #222; border-radius: 5px;");
        m_barres[i] = barre;

        QPushButton *btn = new QPushButton(QString("Laser %1").arg(i + 1));

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
            btn->setIcon(QIcon());
        });
        m_btnLaser[i] = btn;

        QPushButton *assignBtn = new QPushButton("Edit");
        assignBtn->setMinimumHeight(28);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        assignBtn->setStyleSheet(
            "QPushButton { background-color: #222; border-radius: 4px;"
            "  border: 1px solid #444; color: #aaa; font-size: 11px; }"
            "QPushButton:hover { border-color: #ff00ff; color: #ff00ff; }"
            );
        connect(assignBtn, &QPushButton::clicked, this, [=]() {
            toggleAssignation(i);
        });
        m_btnAssign[i] = assignBtn;

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

    for (int i = 0; i < NOTES_DISPLAYED; i++)
    {
        if(!IS_NOIRE[i % 12]) // only draw white ones
        {
            QPushButton *blanc = new QPushButton(NOTES_NAMES[i % 12]);
            blanc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            blanc->setMinimumWidth(10);
            blanc->setStyleSheet(S_BLANCHE);
            connect(blanc, &QPushButton::pressed, this, [=]() {
                m_engine->jouerNoteDirecte(DEFAULT_OCTAVE + i);
            });
            connect(blanc, &QPushButton::released, this, [=]() {
                m_engine->stopperNoteDirecte(DEFAULT_OCTAVE + i);
                if (m_laserEnAssignation >= 0)
                    toggleTouche(DEFAULT_OCTAVE + i);
            });
            m_touchesBlanches.append(blanc);
            pianoLay->addWidget(blanc);
        }
    }

    // Overlay touches noires
    QWidget *blackOverlay = new QWidget(m_pianoWidget);
    blackOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    blackOverlay->setGeometry(0, 0, m_pianoWidget->width(), 75);
    blackOverlay->raise();

    for (int i = 0; i < NOTES_DISPLAYED; i++)
    {
        if(IS_NOIRE[i % 12]) // only draw black ones
        {
            QPushButton *noir = new QPushButton();
            noir->setFixedSize(18, 75);
            noir->setStyleSheet(S_NOIRE);
            noir->setParent(blackOverlay);
            noir->raise();
            m_touchesNoires.append(noir);

            connect(noir, &QPushButton::pressed, this, [=]() {
                m_engine->jouerNoteDirecte(DEFAULT_OCTAVE + i);
            });
            connect(noir, &QPushButton::released, this, [=]() {
                m_engine->stopperNoteDirecte(DEFAULT_OCTAVE + i);
                if (m_laserEnAssignation >= 0)
                    toggleTouche(DEFAULT_OCTAVE + i);
            });
        }
    }

    pianoVBox->addWidget(pianoHint);
    pianoVBox->addWidget(m_pianoWidget);

    centerLayout->addWidget(laserZone);
    centerLayout->addWidget(pianoZone);

    root->addWidget(leftPanel, 20);
    root->addWidget(center, 80);

    // ── Connexions ──
    connect(m_choixInstrument, &QListWidget ::currentRowChanged,
            this, [=](int row) {loadInstrument(row);});

    connect(sliderVol, &QSlider::valueChanged, this, [=](int val) {
        m_engine->setVolume(val / 100.0f); // TODO : use logarithmic scale
        m_configFile->set_volume(val);
    });

    connect(m_engine, &EngineLaser::noteRecueMidi,
            this, [=](int midiNote, bool active) {
                for (int i = 0; i < 6; i++) {
                    m_accords = m_engine->getAccords();
                    for(int i = 0; i < m_accords->n_notes; i++)
                    {
                        if (active) {allumerBarre(midiNote);
                            m_btnLaser[midiNote]->setIcon(QIcon(":/allume.png"));
}

                        else {eteindreBarre(midiNote);
                            m_btnLaser[midiNote]->setIcon(QIcon());
}

                    }
                }
            });

    updateListInstruments(soundFontPath);

    // apply previously loaded settings
    if(m_engine->isAudioOk())
    {
        // load soundFont related stuff only if init was sucessfull
        m_choixInstrument->setCurrentRow(instrumentId);
        m_engine->chargerInstrument(instrumentId);
    }

    sliderVol->setValue(volume);
    m_engine->setVolume(volume / 100.0f);

    m_engine->initMidi(port_id);

    setCentralWidget(central);
    setWindowTitle("Instrument Laser");
    setMinimumSize(700, 480);

    m_engine->setAccords(accords);

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
            "<p><b>Equipe 2 :</b> Lot 2</p>"
            "<hr>"
            "<p>Un instrument de musique laser interactif - "
            "assignez des notes à chaque laser, "
            "choisissez votre instrument et jouez !</p>"
            "<p><i>Amusez-vous bien !</i></p>"

            );
        about.setStandardButtons(QMessageBox::Ok);
        about.exec();
    });
}

void MainWindow::isEngineOk()
{
    while(!m_engine->isAudioOk()) // probably an error with SoundFont loading
    {
        if (QMessageBox::warning(this, "Erreur SoudFont", "Recharger une SoundFont ?", QMessageBox:: Yes | QMessageBox:: No) == QMessageBox::Yes)
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Open SoundFont"), "", tr("SoundFont (*.sf2)"));

            m_engine->initEngine(fileName);

            m_configFile->set_soundFont_path(fileName);
            updateListInstruments(fileName);

            int instrumentId = m_configFile->get_instr_id();
            m_choixInstrument->setCurrentRow(instrumentId);
            m_engine->chargerInstrument(instrumentId);
        }
        else
        {
            break;
        }
    }
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
    int blancIdx = 0;
    for (int idx = 0; idx < NOTES_DISPLAYED; idx++)
    {
        if(IS_NOIRE[idx % 12])
        {
            if (noirIdx >= m_touchesNoires.size()) break;
            if (blancIdx < 0) { noirIdx++; continue;}

            QRect gL = m_touchesBlanches[blancIdx]->geometry();
            QRect gR = m_touchesBlanches[blancIdx - 1]->geometry();
            int xCenter = (gL.right() + gR.left()) / 2;
            m_touchesNoires[noirIdx]->move(xCenter - 9, 0);
            m_touchesNoires[noirIdx]->raise();
            noirIdx++;
        }
        else
        {
            blancIdx++;
        }
    }
}


void MainWindow::resetStylePiano() {
    for (auto t : m_touchesBlanches) t->setStyleSheet(S_BLANCHE);
    for (auto t : m_touchesNoires)   t->setStyleSheet(S_NOIRE);
    for(int i = 0; i < NOTES_DISPLAYED; i++)
    {
        m_pselectedTouches[i] = 0;
    }
}

void MainWindow::toggleTouche(int noteMidi) {
    int noteId = 0;
    int modulo_note = noteMidi % 12;

    if (IS_NOIRE[modulo_note])
    {
        noteId = NOTE_MIDI_TO_ID_TOUCHE_NOIRE[modulo_note];

        if(m_pselectedTouches[noteMidi - DEFAULT_OCTAVE] == 1)
        {
            m_touchesNoires[noteId]->setStyleSheet(S_NOIRE);
            m_pselectedTouches[noteMidi - DEFAULT_OCTAVE] = 0;
        }
        else
        {
            m_touchesNoires[noteId]->setStyleSheet(S_NOIRE_SEL);
            m_pselectedTouches[noteMidi - DEFAULT_OCTAVE] = 1;
        }
    }
    else
    {
        noteId = NOTE_MIDI_TO_ID_TOUCHE_BLANCHE[modulo_note];

        if(m_pselectedTouches[noteMidi - DEFAULT_OCTAVE] == 1)
        {
            m_touchesBlanches[noteId]->setStyleSheet(S_BLANCHE);
            m_pselectedTouches[noteMidi - DEFAULT_OCTAVE] = 0;
        }
        else
        {
            m_touchesBlanches[noteId]->setStyleSheet(S_BLANCHE_SEL);
            m_pselectedTouches[noteMidi - DEFAULT_OCTAVE] = 1;
        }
    }
}

void MainWindow::toggleAssignation(int laserId)
{
    if(m_laserEnAssignation == -1)
    {
        m_laserEnAssignation = laserId;
        m_btnAssign[laserId]->setText("Valider");
        m_btnAssign[laserId]->setStyleSheet(
            "QPushButton { background-color: #00381e; border-radius: 4px;"
            "  border: 1px solid #15fc00; color: #49ec00; font-size: 11px; }"
            );
        resetStylePiano();


        if(m_accords != nullptr)
            delete m_accords;

        m_accords = m_engine->getAccords();
        for(int i = 0; i < m_accords[laserId].n_notes; i++)
        {
            toggleTouche(m_accords[laserId].notes[i]);
        }
    }
    else
    {
        m_btnAssign[laserId]->setText("Edit");
        m_btnAssign[m_laserEnAssignation]->setStyleSheet(
            "QPushButton { background-color: #222; border-radius: 4px;"
            "  border: 1px solid #444; color: #aaa; font-size: 11px; }"
            "QPushButton:hover { border-color: #ff00ff; color: #ff00ff; }"
            );

        m_accords[laserId].n_notes = 0;
        for(int i = 0; i < NOTES_DISPLAYED; i++)
        {
            if(m_pselectedTouches[i] == 1)
            {
                m_accords[laserId].notes[m_accords[laserId].n_notes++] = i + DEFAULT_OCTAVE;
            }
        }
        m_engine->setAccords(m_accords);
        m_configFile->set_accords(m_accords);

        resetStylePiano();
        m_laserEnAssignation = -1;
    }
}

void MainWindow::jouerLaser(int id) {
    m_engine->jouerNote(id);
}

void MainWindow::stopperLaser(int id) {
    m_engine->stopperNote(id);
}

// ─────────────────────────────────────────────
void MainWindow::allumerBarre(int id) {
    m_accords = m_engine->getAccords();

    for(int i = 0; i < m_accords[id].n_notes; i++)
    {

        int noteMidi = m_accords[id].notes[i];
        int noteId = 0;

        int modulo_note = noteMidi % 12;

        if (IS_NOIRE[modulo_note])
        {
            noteId = NOTE_MIDI_TO_ID_TOUCHE_NOIRE[modulo_note];
            m_touchesNoires[noteId]->setStyleSheet(S_NOIRE_PLAY);
        }
        else
        {
            noteId = NOTE_MIDI_TO_ID_TOUCHE_BLANCHE[modulo_note];
            m_touchesBlanches[noteId]->setStyleSheet(S_BLANCHE_PLAY);
        }
    }


    m_barres[id]->setStyleSheet(
        "background-color: #ff00ff; border-radius: 5px;");
}

void MainWindow::eteindreBarre(int id) {
    m_barres[id]->setStyleSheet(
        "background-color: #222; border-radius: 5px;");
    resetStylePiano();
}

// ─────────────────────────────────────────────
void MainWindow::fermer() {
    if (QMessageBox::warning(this, "Quitter", "Fermer l'application ?", QMessageBox:: Yes | QMessageBox:: No)
        == QMessageBox::Yes)
    {
        m_configFile->write(m_configPath);
        close();
    }
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open SoundFont"), "", tr("SoundFont (*.sf2)"));

    if(!m_engine->isAudioOk()) // there was probably an error on initialisation with the last SoundFont
        m_engine->initEngine(fileName);

    m_configFile->set_soundFont_path(fileName);
    updateListInstruments(fileName);
}

void MainWindow::updateListInstruments(QString& fileName)
{
    m_engine->loadSoundFont(fileName);

    m_choixInstrument->clear();

    // Remplir QlistWidget avec instruments SF2
    QStringList instruments = m_engine->getInstrumentsDisponibles();
    if (!instruments.isEmpty()) {
        m_choixInstrument->blockSignals(true);
        m_choixInstrument->addItems(instruments);
        m_choixInstrument->blockSignals(false);
    } else {
        m_choixInstrument->addItems({"Pas de SoundFont chargee"});
    }

    // load the fisrt instrument in the SoundFont
    m_engine->chargerInstrument(0);
    m_configFile->set_instr_id(0);
    m_choixInstrument->setCurrentRow(0);
}

void MainWindow::connectMidi(int id)
{
    m_engine->stopMidi();
    m_engine->initMidi(id);
    m_configFile->set_port_id(id);
}

void MainWindow::updatePorts()
{
    if(!mListePeripheriques->isEmpty())
        mListePeripheriques->clear();

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

void MainWindow::loadInstrument(int id)
{
    m_engine->chargerInstrument(id);
    m_configFile->set_instr_id(id);
}

void MainWindow::loadConfig()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open configuration file"), "", tr("Configuration file (*.json)"));
    delete m_configFile;
    m_configFile = new configFile(fileName);
    m_configPath = fileName;

    QString soundFontPath = m_configFile->get_soundFont_path();
    int instrumentId = m_configFile->get_instr_id();
    int volume = m_configFile->get_volume();
    // int port_id = m_configFile->get_port_id(); we don't change the midi port
    accord_t* accords = m_configFile->getAccords();

    updateListInstruments(soundFontPath);
    m_choixInstrument->setCurrentRow(instrumentId);
    m_engine->chargerInstrument(instrumentId);

    sliderVol->setValue(volume);
    m_engine->setVolume(volume / 100.0f);

    m_engine->setAccords(accords);
}

void MainWindow::saveConfig()
{
    m_configFile->write(m_configPath);
}

void MainWindow::saveConfigAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save configuration file"), "config.json", tr("Configuration file (*.json)"));
    m_configFile->write(fileName);
    m_configPath = fileName;
}
