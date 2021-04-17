#include "mainwindow.h"
#include "Compiler.h"
#include <QApplication>
#include <iostream>


using namespace std;
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