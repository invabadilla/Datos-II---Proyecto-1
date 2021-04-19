//
// Created by usuario on 3/29/21.
//

#ifndef PROYECTO1_COMPILER_H
#define PROYECTO1_COMPILER_H


#include <string>
#include <QString>
#include <QStringList>
using namespace std;

class Compiler {
public:
    inline static std::string std_out = "<< \n";
    inline static std::string log= "<< \n";
    inline static std::string ram ="<< \n";
    inline static int port;

    static void updateStrings(string std_out, string log, string ram);
    static string updateGUI();
    void compile(QString line);
    void sendServer();

    bool validename(std::string name);
    bool validNum(std::string num);
    string valideReference(std::string nom);

    QStringList Divide(QStringList initial);



};


#endif //PROYECTO1_COMPILER_H
