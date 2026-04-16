#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow fenetre;

    fenetre.show();
    fenetre.isEngineOk();

    return app.exec();
}
