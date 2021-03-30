//
// Created by usuario on 3/29/21.
//

#include "Compiler.h"
#include <string>
#include <iostream>
#include <QString>
#include <QStringList>


using namespace std;



void Compiler::compile(QString line) {
    QStringList words = line.split(" ", QString::SkipEmptyParts);
    if ("int" == words.at(0).toStdString()){
        cout <<"soy un int\n";
        if (validename(words.at(1).toStdString())){
            cout  <<"nombre correcto\n";
            string name = words.at(1).toStdString();
            if (name[name.length()-1] == ';'){
                cout  <<"declaracion de variable vacia\n";
            }
            else if (words.at(2).toStdString() == "="){
                if (words.length() == 4){
                    string value = words.at(3).toStdString().erase(words.at(3).toStdString().length()-1);
                    try{
                        if (value.length() == to_string(stoi(value)).length()){
                            cout  <<"declaracion de variable con un valor de: " + value + "\n";
                        }else{ cout  <<"tipo no coincide con valor\n";}
                    }catch (std::invalid_argument){
                        cout  <<"tipo no coincide con valor\n";
                    }
                }
                else{
                    if (words.at(4).toStdString() == ";" && words.length() == 5){
                        string value = words.at(3).toStdString();
                        try{
                            if (value.length() == to_string(stoi(value)).length()){
                                cout  <<"declaracion de variable con un valor de: " + value + "\n";
                            }else{ cout  <<"tipo no coincide con valor\n";}
                        }catch (std::invalid_argument){
                            cout  <<"tipo no coincide con valor\n";
                        }
                    }
                    else{
                        cout  <<"error cosas escritas luego del ;\n";
                    }
                }
            }
            else{
                cout <<"error sintaxis\n";
            }

        }
        else{
            cout  <<"nombre incorrecto\n";
        }
    }

    else if("long" == words.at(0).toStdString()){
        cout <<"soy un long\n";
        if (validename(words.at(1).toStdString())){
            cout  <<"nombre correcto\n";
        }
        else{
            cout  <<"nombre incorrecto\n";
        }
    }
    else if("char" == words.at(0).toStdString()){
        cout <<"soy un char\n";
        if (validename(words.at(1).toStdString())){
            cout  <<"nombre correcto\n";
        }
        else{
            cout  <<"nombre incorrecto\n";
        }
    }
    else if("float" == words.at(0).toStdString()){
        cout <<"soy un float\n";
        if (validename(words.at(1).toStdString())){
            cout  <<"nombre correcto\n";
        }
        else{
            cout  <<"nombre incorrecto\n";
        }
    }
    else if("double" == words.at(0).toStdString()){
        cout <<"soy un double\n";
        if (validename(words.at(1).toStdString())){
            cout  <<"nombre correcto\n";
        }
        else{
            cout  <<"nombre incorrecto\n";
        }
    }
    /**
    else if("struct" == words.at(0).toStdString()){
        cout <<"soy un struct\n";
        if (validename(words.at(1).toStdString())){
            cout  <<"nombre correcto\n";
        }
        else{
            cout  <<"nombre incorrecto\n";
        }
    }

    else if("reference" == words.at(0).toStdString()){
        cout <<"soy un reference\n";
        if (validename(words.at(1).toStdString())){
            cout  <<"nombre correcto\n";
        }
        else{
            cout  <<"nombre incorrecto\n";
        }
    }**/
    else{
        cout <<"error\n";
    }
}

bool Compiler::validename(string name){
    if (name != "int" && name != "long" && name != "char" && name != "float" && name != "double" && name != "struct" && name != "reference"){
        if ((name[0] == '_') || (isalpha(name[0]))){
            for (int i=1; i < name.length();i++){
                if (isalpha(name[i]) || name[i] == '_' || isdigit(name[i])){
                    continue;
                }
                else if (i+1 == name.length() && name[i] == ';'){
                    return true;
                }
                else{return false;}
            }return true;
        }else{return false;}
    }else{return false;}
}

void Compiler::sendServer() {

}
