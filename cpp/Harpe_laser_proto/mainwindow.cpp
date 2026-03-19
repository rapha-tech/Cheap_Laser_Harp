#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define N_LASERS 6
#define SPACE 156

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene_bars = new QGraphicsScene;
    ui->graphicsView->setScene(scene_bars);
    draw_bars();

    QGraphicsScene *scene_piano = new QGraphicsScene;
    ui->graphicsView_piano->setScene(scene_piano);
    piano = new Piano_ui(scene_piano);

    QObject::connect(ui->push_button_0, SIGNAL(clicked()), this, SLOT(button_pressed()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete piano;
}

void MainWindow::draw_bars()
{
    //int width = ui->graphicsView->viewport()->width();

    //int space = width/N_LASERS;
    pRects = new QGraphicsRectItem*[N_LASERS];

    for(int i = 0; i<N_LASERS; i++)
    {
        pRects[i] = scene_bars->addRect(QRectF(i * SPACE, 0, 10, 200), QPen(Qt::NoPen), QBrush(Qt::red)); // QPen for border, QBrush for fill
    }
}

void MainWindow::set_bar(int bar_id, bool state)
{
    if(bar_id < N_LASERS && bar_id >= 0) // clipping for now
    {
        QBrush brush(Qt::red);
        if(state == 1)
            brush = QBrush(Qt::green);

        pRects[bar_id]->setBrush(brush);
    }
}

void MainWindow::add_instrument(const char* instrument)
{
    ui->listWidget_1->setCurrentRow(0);
    ui->listWidget_1->addItem(instrument);
}

int MainWindow::get_chosen_instrument()
{
    int choice = ui->listWidget_1->currentRow();
    return choice;
}

void MainWindow::button_pressed()
{
    int* liste_note = piano->get_note_state();
    int sum = 0;
    for(int i = 0; i<12*1; i++)
    {
        if(liste_note[i] == 1)
            sum += 1;

    }
    delete liste_note;
    ui->label->setNum(sum);
}