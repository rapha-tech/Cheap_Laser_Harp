#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // Initialise les ressources de l'application (indispensable pour Qt)
    QApplication app(argc, argv);

    // Crée l'instance de ta fenêtre (qui contient ton moteur EngineLaser)
    MainWindow fenetre;

    // Affiche la fenêtre à l'écran
    fenetre.show();

    // Lance la boucle d'événements (attend les clics ou signaux laser)
    return app.exec();
}
