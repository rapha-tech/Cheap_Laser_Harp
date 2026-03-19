#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include "piano_ui.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void button_pressed();


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void add_instrument(const char*);
    int get_chosen_instrument();
    void set_bar(int bar_id, bool state);

private:
    QGraphicsScene* scene;
    QGraphicsRectItem** pRects;
    Ui::MainWindow *ui;
    Piano_ui* piano;


    void draw_bars();

};
#endif // MAINWINDOW_H
