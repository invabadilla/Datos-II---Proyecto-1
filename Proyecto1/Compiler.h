//
// Created by usuario on 3/29/21.
//

#ifndef PROYECTO1_COMPILER_H
#define PROYECTO1_COMPILER_H


#include <string>
#include <QString>
#include <QStringList>
#include <vector>
using namespace std;

/**
 * Clase encargada de la compilacion y ejecucion de las lineas de codigo
 */
class Compiler {
public:
    inline static std::string std_out = "<< \n"; //Almacena los prints que hace el codigo
    inline static std::string log= "<< \n"; //Almacena el loggin del sistema
    inline static std::string ram ="<< \n"; //Almacena la informacion de datos almacenados en la ram
    inline static int port; //Puerto donde se conecta con el mServer
    inline static vector<string> s_existing; //Lista que guarda los nombres reservados de los Struct

    static void updateStrings(string std_out, string log, string ram);
    static string updateGUI();
    void compile(QString line);

    bool validenamestruct(string name);
    bool validename(std::string name);
    string valideReference(std::string nom);

    QStringList Divide(QStringList initial);

};

#endif //PROYECTO1_COMPILER_H
