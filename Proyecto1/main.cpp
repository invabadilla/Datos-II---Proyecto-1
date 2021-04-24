#include "mainwindow.h"
#include "Compiler.h"
#include <QApplication>
#include <iostream>

using namespace std;
/**
 * Clase encargada de inicializar el GUI y solicitar el puerto del mServer
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char *argv[])
{
    cout<< "Ingrese el puerto de escucha del server: ";
    string port;
    cin >> port; //Se lee el nombre
    Compiler *compiler = new Compiler();
    compiler->port = stoi(port);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}