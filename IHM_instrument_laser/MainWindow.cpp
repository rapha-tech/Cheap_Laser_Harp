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
#include <QList>
#include <QScrollArea>
#include <QScrollBar>

// command to make executable D:\Qt\6.11.0\mingw_64\bin\windeployqt D:\Cheap_Laser_Harp\IHM_instrument_laser\build\Desktop_Qt_6_11_0_MinGW_64_bit-Release\release

const QStringList MainWindow::NOTES_NAMES = {
    "Do","Do#","Re","Re#","Mi","Fa","Fa#","Sol","Sol#","La","La#","Si"
};
const QList<int> MainWindow::IS_NOIRE = {0,1,0,1,0,0,1,0,1,0,1,0};

static const QString S_BLANCHE =
    "QPushButton {"
    "  background-color: #FFFFFF;"
    "  border: 1px solid #000000;"
    "  color: black;"
    "  text-align: bottom center;"
    "  padding-bottom: 20px;"
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
    "  color: black;"
    "  text-align: bottom center;"
    "  padding-bottom: 20px;"
    "  font-size: 9px;"
    "}";

static const QString S_BLANCHE_PLAY =
    "QPushButton {"
    " background-color: #ff00ff;" // violet
    " border: 1px solid #000000;" // contour noir
    "  color: black;"
    "  text-align: bottom center;"
    "  padding-bottom: 20px;"
    "  font-size: 9px;"
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
    for(int i = 0; i < NOTES_TOTAL; i++)
    {
        noteIdNoire = 0;
        noteIdBlanche = 0;
        for(int j = 0; j < i; j++)
        {
            if(IS_NOIRE[j % 12])
                noteIdNoire++;
            else
                noteIdBlanche++;
        }
        if(IS_NOIRE[i % 12])
            NOTE_MIDI_TO_ID_TOUCHE_NOIRE[DEFAULT_OCTAVE + i] = noteIdNoire;
        else
            NOTE_MIDI_TO_ID_TOUCHE_BLANCHE[DEFAULT_OCTAVE + i] = noteIdBlanche;
    }

    m_engine = new EngineLaser(this);

    QMenu *mFichier = menuBar()->addMenu("&Fichier");
    QAction *actionnv = new QAction("&Nouveau projet", this);
    mFichier->addAction(actionnv);
    actionnv->setShortcut(QKeySequence(tr("Ctrl + N")));
    connect(actionnv, SIGNAL(triggered()), this, SLOT(nouveau()));

    QAction *actionCharger = new QAction("&Ouvrir une configuration...", this);
    mFichier->addAction(actionCharger);
    connect(actionCharger, SIGNAL(triggered()), this, SLOT(getConfigPathLoad()));

    mListeLatestConfigs = new QMenu("&Configurations récentes");
    mFichier->addMenu(mListeLatestConfigs);

    connect(mFichier, &QMenu::aboutToShow, this, [=]() {
        updateLatestConfigs();
    });

    QAction *actionSave = new QAction("&Enregistrer", this);
    mFichier->addAction(actionSave);
    actionSave->setShortcut(QKeySequence(tr("Ctrl + S")));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(saveConfig()));

    QAction *actionSaveAs = new QAction("&Enregistrer sous...", this);
    mFichier->addAction(actionSaveAs);
    connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(saveConfigAs()));

    mFichier->addSeparator();

    QAction *actionQuitter = new QAction("&Quitter", this);
    mFichier->addAction(actionQuitter);
    actionQuitter->setShortcut(QKeySequence(tr("Ctrl + Q")));
    connect(actionQuitter, SIGNAL(triggered()), this, SLOT(close()));



    QMenu *mInstrument = new QMenu("&Son");

    QAction *actionSF2 = new QAction("&Charger une SoundFont...", this);
    mInstrument->addAction(actionSF2);
    connect(actionSF2, SIGNAL(triggered()), this, SLOT(getSoundFontPath()));

    mListeLatestSoundFont = new QMenu("&SoundFont récentes");
    mInstrument->addMenu(mListeLatestSoundFont);
    connect(mInstrument, &QMenu::aboutToShow, this, [=]() {
        updateLatestSoundFonts();
    });

    mListePeripheriques = new QMenu("&Périphérique Midi entrant");
    mInstrument->addMenu(mListePeripheriques);
    menuBar()->addMenu(mInstrument);
    // this action is used to update the list of available Midi ports when the menu bar is triggered
    connect(mInstrument, &QMenu::aboutToShow, this, [=]() {
        updatePorts();
    });

    mListeAudioOut = new QMenu("&Sortie Audio");
    mInstrument->addMenu(mListeAudioOut);
    connect(mInstrument, &QMenu::aboutToShow, this, [=]() {
        updateAudioOuts();
    });


    QMenu *mLasers = menuBar()->addMenu("&Lasers");
    QAction *reinitialiser = new QAction("&Réinitialiser les assignations", this);
    mLasers->addAction(reinitialiser);
    reinitialiser->setShortcut(QKeySequence(tr("Ctrl + R")));

    connect(reinitialiser,
            &QAction::triggered, this, [=]() {
                for (int i = 0; i < 6; i++) {
                    SAFE_DELETE(m_accords);
                    m_accords = m_configFile->get_default_accord();
                    m_engine->setAccords(m_accords);
                }
            });


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

    QAction *actionAfficherNotes = mAffichage->addAction("&Afficher les notes sur les touches");
    connect(actionAfficherNotes, &QAction::triggered, this, [=]() {
        toggleNotes();
    });
    // TODO : make function

    QAction *actionTheme = mAffichage->addAction("&[TODO]Changer de thème");
    // TODO : make function


    QMenu *mAide = menuBar()->addMenu("&Aide");

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

    connect(mAide->addAction("&Guide démarrage rapide"), &QAction::triggered, this, [=]() {
        QMessageBox guide(this);
        guide.setWindowTitle("Guide démarrage rapide");
        guide.setIcon(QMessageBox::Information);
        guide.setText(
            "<h3>TODO</h3>"
            );
        guide.setStandardButtons(QMessageBox::Ok);
        guide.exec();
    });


    // ── Layout principal ──
    QWidget *central = new QWidget(this);
    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(10);

leftPanel   = new QWidget();
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
    left->addStretch();


    QPushButton *rendreactif = new QPushButton("Rendre Actif");
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
            btn->setIcon(QIcon(":/allume.png"));

        });
        connect(btn, &QPushButton::released, this, [=]() {
            stopperLaser(i);
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

    QVBoxLayout *pianoWidgetLay = new QVBoxLayout(m_pianoWidget);
    pianoWidgetLay->setContentsMargins(0, 0, 0, 0);
    pianoWidgetLay->setSpacing(0);

    const int WHITE_KEY_WIDTH = 28;
    const int WHITE_KEY_SPACING = 1;
    const int BLACK_KEY_WIDTH = 18;
    const int BLACK_KEY_HEIGHT = 75;
    const int PIANO_HEIGHT = 120;

    // Count white keys to compute total width
    int whiteKeyCount = 0;
    for (int i = 0; i < NOTES_TOTAL; i++)
        if (!IS_NOIRE[i % 12]) whiteKeyCount++;

    const int TOTAL_WIDTH = whiteKeyCount * (WHITE_KEY_WIDTH + WHITE_KEY_SPACING);

pianoScroll = new QScrollArea();
    pianoScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    pianoScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pianoScroll->setWidgetResizable(false);
    pianoScroll->setFrameShape(QFrame::NoFrame);

pianoCanvas = new QWidget();
    pianoCanvas->setMinimumSize(TOTAL_WIDTH, PIANO_HEIGHT);
    pianoScroll->setMinimumHeight(PIANO_HEIGHT + 18);

    QHBoxLayout *pianoLay = new QHBoxLayout(pianoCanvas);
    pianoLay->setSpacing(WHITE_KEY_SPACING);
    pianoLay->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < NOTES_TOTAL; i++)
    {
        if (!IS_NOIRE[i % 12])
        {
            QPushButton *blanc = new QPushButton(NOTES_NAMES[i % 12]);
            blanc->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
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

    int whiteIndex = 0;
    for (int i = 0; i < NOTES_TOTAL; i++)
    {
        if (!IS_NOIRE[i % 12])
        {
            whiteIndex++;
        }
        else
        {
            int x = whiteIndex * (WHITE_KEY_WIDTH + WHITE_KEY_SPACING)
            - (BLACK_KEY_WIDTH / 2) - WHITE_KEY_SPACING;

            QPushButton *noir = new QPushButton(pianoCanvas);
            noir->setFixedSize(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT);
            noir->move(x, 0);
            noir->setStyleSheet(S_NOIRE);
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

    pianoScroll->setWidget(pianoCanvas);
    pianoWidgetLay->addWidget(pianoScroll);

    pianoVBox->addWidget(pianoHint);
    pianoVBox->addWidget(m_pianoWidget);

    centerLayout->addWidget(laserZone);
    centerLayout->addWidget(pianoZone);

    root->addWidget(leftPanel, 25);
    root->addWidget(center, 75);

    // ── Connexions ──
    connect(m_choixInstrument, &QListWidget ::currentRowChanged,
            this, [=](int row) {loadInstrument(row, false);});

    connect(sliderVol, &QSlider::valueChanged, this, [=](int val) {
        setVolume(val, false);
    });

    connect(m_engine, &EngineLaser::noteRecueMidi,
            this, [=](int index_touche, bool active) {
                if (active)
                {
                    allumerBarre(index_touche);
                    m_btnLaser[index_touche]->setIcon(QIcon(":/allume.png"));
                }
                else
                {
                    eteindreBarre(index_touche);
                    m_btnLaser[index_touche]->setIcon(QIcon());
                }
            });

    setCentralWidget(central);
    setWindowTitle("Instrument Laser");
    setMinimumSize(700, 480);

    // load RecentFiles file
    m_recentFiles = new RecentFiles;
    QStringList recentConfigs = m_recentFiles->getListConfigs();

    if(recentConfigs.isEmpty())
    {
        // load default config
        QString default_configPath = QString(DEFAULT_CONFIG_PATH);
        loadConfig(default_configPath);
    }
    else
    {
        // load latest config file
        loadConfig(recentConfigs[0]);
    }


    QTimer::singleShot(0, [=]() {
        QScrollBar *hbar = pianoScroll->horizontalScrollBar();
        hbar->setValue((hbar->minimum() + hbar->maximum()) / 2);
    });

    connect(actionTheme, &QAction::triggered, this, [=]() {
        m_darkMode = !m_darkMode;
        actionTheme->setText(m_darkMode ? "&Passer en mode clair" : "&Passer en mode sombre");
        applyTheme();
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
            loadSoundFont(fileName, 0);
        }
        else
        {
            break;
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Q:
        jouerLaser(0);
        break;

    case Qt::Key_S:
        jouerLaser(1);
        break;

    case Qt::Key_D:
        jouerLaser(2);
        break;

    case Qt::Key_F:
        jouerLaser(3);
        break;

    case Qt::Key_G:
        jouerLaser(4);
        break;

    case Qt::Key_H:
        jouerLaser(5);
        break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Q:
        stopperLaser(0);
        break;

    case Qt::Key_S:
        stopperLaser(1);
        break;

    case Qt::Key_D:
        stopperLaser(2);
        break;

    case Qt::Key_F:
        stopperLaser(3);
        break;

    case Qt::Key_G:
        stopperLaser(4);
        break;

    case Qt::Key_H:
        stopperLaser(5);
        break;
    }
}

void MainWindow::toggleNotes()
{
    if(m_touchesBlanches[0]->text() == "")
    {
        int noteId = 0;

        for(int i = 0; i < NOTES_TOTAL; i++)
        {
            if(!IS_NOIRE[i % 12])
            {
                noteId = NOTE_MIDI_TO_ID_TOUCHE_BLANCHE[i + DEFAULT_OCTAVE];
                m_touchesBlanches[noteId]->setText(NOTES_NAMES[i % 12]);

            }
        }
    }
    else
    {
        for(int i = 0; i < m_touchesBlanches.size(); i++)
            m_touchesBlanches[i]->setText("");
    }
}

void MainWindow::resetStylePiano()
{
    for(int i = 0; i < m_touchesBlanches.size(); i++)
        m_touchesBlanches[i]->setStyleSheet(S_BLANCHE);

    for(int i = 0; i < m_touchesNoires.size(); i++)
        m_touchesNoires[i]->setStyleSheet(S_NOIRE);

    for(int i = 0; i < NOTES_TOTAL; i++)
    {
        m_pselectedTouches[i] = 0;
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!leftPanel || !pianoScroll || !pianoCanvas) return;

    int w = event->size().width();
    int h = event->size().height();

    // Panneau gauche
    if (w > 1200)
        leftPanel->setFixedWidth(w * 0.15);
    else
        leftPanel->setFixedWidth(w * 0.20);


    for (auto *b : m_barres)
        b->setFixedWidth(qMax(8, w / 80));

    for (auto *b : m_btnLaser)
        b->setMinimumHeight(h * 0.22);

    for (auto *b : m_btnAssign)
        b->setMinimumHeight(h * 0.04);

    int pianoH = h * 0.30;
    pianoScroll->setFixedHeight(pianoH);

    // blanche
    int ww = qMax(10, (pianoCanvas->width() / m_touchesBlanches.size()) - 1);
    for (auto *b : m_touchesBlanches)
        b->setFixedWidth(ww);

    // noire
    int bw = ww * 0.65;
    int bh = (pianoH - 18) * 0.62;
    int whiteIndex = 0;
    int noireIdx = 0;
    for (int i = 0; i < NOTES_TOTAL; i++)
    {
        if (!IS_NOIRE[i % 12])
            whiteIndex++;
        else
        {
            int x = whiteIndex * (ww + 1) - bw / 2;
            m_touchesNoires[noireIdx]->setFixedSize(bw, bh);
            m_touchesNoires[noireIdx]->move(x, 0);
            noireIdx++;
        }
    }
    int totalWidth = m_touchesBlanches.size() * (ww + 1);
    pianoCanvas->setFixedSize(totalWidth, pianoH - 18);
}
#include <QApplication>

void MainWindow::applyTheme()
{
    if (m_darkMode)
    {
        qApp->setStyleSheet("");  // remet tout par défaut
        // dans le if (m_darkMode)
        for (int i = 0; i < 6; i++)
            m_barres[i]->setStyleSheet("background-color: #222; border-radius: 5px;");

        for (auto *b : m_btnLaser) b->setStyleSheet(
                "QPushButton { background-color: #333; border-radius: 8px; color: #aaa; }"

                );
    }
    else
    {
        qApp->setStyleSheet(
            "QWidget { background-color: #f0f0f0; color: #111111; }"
            "QMenuBar { background-color: #e0e0e0; }"
            "QMenu { background-color: #e0e0e0; }"
            "QListWidget { background-color: #ffffff; color: #111111; }"


            );
        for (auto *b : m_barres)   b->setStyleSheet("background-color: #d0d0d0; border-radius: 5px;");
        for (auto *b : m_btnLaser) b->setStyleSheet(
                "QPushButton { background-color: #e0e0e0; border-radius: 8px; color: #111; }"

                );
    }
}
void MainWindow::toggleTouche(int noteMidi)
{
    int noteId = 0;

    if (IS_NOIRE[noteMidi % 12])
    {
        noteId = NOTE_MIDI_TO_ID_TOUCHE_NOIRE[noteMidi];

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
        noteId = NOTE_MIDI_TO_ID_TOUCHE_BLANCHE[noteMidi];

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

        SAFE_DELETE(m_accords);
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
        for(int i = 0; i < NOTES_TOTAL; i++)
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

void MainWindow::jouerLaser(int id)
{
    m_engine->jouerLaser(id);
    allumerBarre(id);
}

void MainWindow::stopperLaser(int id)
{
    m_engine->stopperLaser(id);
    eteindreBarre(id);
}

// ─────────────────────────────────────────────
void MainWindow::allumerBarre(int id)
{
    for(int i = 0; i < m_accords[id].n_notes; i++)
    {

        int noteMidi = m_accords[id].notes[i];
        int noteId = 0;

        if (IS_NOIRE[noteMidi % 12])
        {
            noteId = NOTE_MIDI_TO_ID_TOUCHE_NOIRE[noteMidi];
            m_touchesNoires[noteId]->setStyleSheet(S_NOIRE_PLAY);
        }
        else
        {
            noteId = NOTE_MIDI_TO_ID_TOUCHE_BLANCHE[noteMidi];
            m_touchesBlanches[noteId]->setStyleSheet(S_BLANCHE_PLAY);
        }
    }


    m_barres[id]->setStyleSheet(
        "background-color: #ff00ff; border-radius: 5px;");
}

void MainWindow::eteindreBarre(int id)
{

    QString couleur = m_darkMode ? "#222" : "#d0d0d0";
    m_barres[id]->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(couleur));

    resetStylePiano();
}

// ─────────────────────────────────────────────
void MainWindow::fermer() {
    if (QMessageBox::warning(this, "Quitter", "Fermer l'application ?", QMessageBox:: Yes | QMessageBox:: No)
        == QMessageBox::Yes)
    {
        m_configFile->write(m_configPath);

        m_recentFiles->addListConfigs(m_configPath);
        m_recentFiles->write();

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

void MainWindow::getSoundFontPath()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open SoundFont"), "", tr("SoundFont (*.sf2)"));
    loadSoundFont(fileName);
}

void MainWindow::loadSoundFont(QString& soundFontPath, int instrumentId)
{
    if(soundFontPath != m_soundFontPath && soundFontPath != QString())
    {
        // completely restart the engine (probably not necessary but safer)
        m_engine->initEngine(soundFontPath);

        if(m_engine->isAudioOk()) // only save the soundFont if it is a valid one
        {
            m_configFile->set_soundFont_path(soundFontPath);

            // update latest SoundFont
            m_recentFiles->addListSoundFonts(soundFontPath);
            m_recentFiles->write();

            m_soundFontPath = soundFontPath;
        }
        else
        {
            // the engine failed to load the new soundFont, fallback to the old one
            m_engine->initEngine(m_soundFontPath);
        }

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

        loadInstrument(instrumentId, true);
    }
}

void MainWindow::loadInstrument(int id, bool setRow)
{
    if((id != m_instrumentId) && (id >= 0))
    {
        m_instrumentId = id;
        if(setRow)
            m_choixInstrument->setCurrentRow(id);

        m_engine->chargerInstrument(id);
        m_configFile->set_instr_id(id);
    }
}

void MainWindow::setVolume(int val, bool slider)
{
    if(val != m_volume)
    {
        m_volume = val;
        if(slider)
            sliderVol->setValue(val);
        m_engine->setVolume(val / 100.0f);
        m_configFile->set_volume(val);
    }
}

void MainWindow::connectMidi(int id)
{
    m_engine->stopMidi();
    m_engine->initMidi(m_listePortsMidi[id]);
    m_configFile->set_port_name(m_listePortsMidi[id]);
}

void MainWindow::updatePorts()
{
    if(!mListePeripheriques->isEmpty())
        mListePeripheriques->clear();

    // On ajouter les périphériques disponibles (connectés au PC)
    m_listePortsMidi = m_engine->getMidiPorts();

    if(m_listePortsMidi.size() > 0)
    {
        for(unsigned int i = 0; i < m_listePortsMidi.size(); i++)
        {
            QAction* actionsPort = new QAction(m_listePortsMidi[i], this);
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

void MainWindow::updateAudioOuts()
{
    if(!mListeAudioOut->isEmpty())
        mListeAudioOut->clear();

    QStringList ListeAudioOut = m_engine->getAudioOuts();

    if(ListeAudioOut.size() > 0)
    {
        for(unsigned int i = 0; i < ListeAudioOut.size(); i++)
        {
            QAction* actionsAudioOut = new QAction(ListeAudioOut[i], this);
            mListeAudioOut->addAction(actionsAudioOut);
            connect(actionsAudioOut, &QAction::triggered, this, [=]() {
                m_engine->initEngine(m_soundFontPath, i);
                // FIXME : we might have to reinit midi ?
            });
        }
    }
    else
    {
        mListePeripheriques->addAction("&Aucune sortie audio");
    }
}

void MainWindow::updateLatestConfigs()
{
    if(!mListeLatestConfigs->isEmpty())
        mListeLatestConfigs->clear();

    QStringList recentFiles = m_recentFiles->getListConfigs();

    if(recentFiles.size() > 0)
    {
        for(unsigned int i = 0; i < recentFiles.size(); i++)
        {
            // TODO : print only the filename (not the full path)
            QAction* actionFile = new QAction(recentFiles[i], this);
            mListeLatestConfigs->addAction(actionFile);
            connect(actionFile, &QAction::triggered, this, [=]() {
                QString configPath(recentFiles[i]);
                loadConfig(configPath);
            });
        }
    }
    else
    {
        mListeLatestConfigs->addAction("&Aucune config recente");
    }
}

void MainWindow::updateLatestSoundFonts()
{
    if(!mListeLatestSoundFont->isEmpty())
        mListeLatestSoundFont->clear();

    QStringList recentSoundFonts = m_recentFiles->getListSoundFonts();
    if(recentSoundFonts.size() > 0)
    {
        for(unsigned int i = 0; i < recentSoundFonts.size(); i++)
        {
            // TODO : print only the filename (not the full path)
            QAction* actionFile = new QAction(recentSoundFonts[i], this);
            mListeLatestSoundFont->addAction(actionFile);
            connect(actionFile, &QAction::triggered, this, [=]() {
                QString soundFontPath(recentSoundFonts[i]);
                loadSoundFont(soundFontPath);
            });
        }
    }
    else
    {
        mListeLatestSoundFont->addAction("&Aucune SoundFont recente");
    }
}

void MainWindow::getConfigPathLoad()
{
    QString configPath = QFileDialog::getOpenFileName(this, tr("Open configuration file"), "", tr("Configuration file (*.json)"));
    if(configPath != QString())
        loadConfig(configPath);
}

void MainWindow::loadConfig(QString& configPath)
{
    if(configPath != m_configPath)
    {
        SAFE_DELETE(m_configFile);
        m_configFile = new configFile(configPath);
        m_configPath = configPath;

        QString soundFontPath = m_configFile->get_soundFont_path();
        int instrumentId = m_configFile->get_instr_id();
        int volume = m_configFile->get_volume();
        QString midi_port_name = m_configFile->get_port_name();

        loadSoundFont(soundFontPath, instrumentId);
        setVolume(volume, true);

        m_engine->initMidi(midi_port_name);

        SAFE_DELETE(m_accords);
        m_accords = m_configFile->getAccords();
        m_engine->setAccords(m_accords);

        // update latest config
        m_recentFiles->addListConfigs(configPath);
        m_recentFiles->write();
    }
}

void MainWindow::saveConfig()
{
    m_configFile->write(m_configPath);

    m_recentFiles->addListConfigs(m_configPath);
    m_recentFiles->write();
}

void MainWindow::saveConfigAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save configuration file"), "config.json", tr("Configuration file (*.json)"));
    m_configFile->write(fileName);
    m_configPath = fileName;

    m_recentFiles->addListConfigs(fileName);
    m_recentFiles->write();
}
