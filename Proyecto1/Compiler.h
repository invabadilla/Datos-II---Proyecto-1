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
    static std::string std_out;
    static std::string log;
    static std::string ram;

    void updateStrings(string std_out, string log, string ram);
    string updateGUI();
    void compile(QString line);
    void sendServer();

    bool validename(std::string name);
    bool validNum(std::string num);

    QStringList Divide(QStringList initial);



};


#endif //PROYECTO1_COMPILER_H
