//
// Created by usuario on 3/29/21.
//

#ifndef PROYECTO1_COMPILER_H
#define PROYECTO1_COMPILER_H


#include <string>
#include <QString>

class Compiler {
public:
    void compile(QString line);
    void sendServer();

    bool validename(std::string name);
};


#endif //PROYECTO1_COMPILER_H
