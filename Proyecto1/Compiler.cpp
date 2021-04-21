//
// Created by usuario on 3/29/21.
//

#include <iomanip>
#include "Compiler.h"
#include <string>
#include <iostream>
#include <QString>
#include <QStringList>
#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <thread>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <sstream>
#include <utility>
#include "json.hpp"
#include <iomanip>

using namespace std;
using json = nlohmann::json;

int startClient(json message) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1){
        return 1;
    }

    //	Create a hint structure for the server we're connecting with
    int port = Compiler::port;
    string ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    //	Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1){
        cout << "Could not send to server! Whoops!\r\n";
        return 1;
    }

    //	While loop:
    char buf[4096];
    string userInput;
    string mymessage = message.dump();
    cout << mymessage << endl;
    //		Send to server
    int sendRes = send(sock, mymessage.c_str(), mymessage.size() + 1, 0);
    if (sendRes == -1){
        cout << "Could not send to server! Whoops!\r\n";
    }

    //		Wait for response
    memset(buf, 0, 4096);
    int bytesReceived = recv(sock, buf, 4096, 0);
    if (bytesReceived == -1){
        cout << "There was an error getting response from server\r\n";
    }
    else{
        string messageR = string(buf, bytesReceived);
        if(messageR != "") {
            json jmessageR = json::parse(messageR);
            string std_out_ = jmessageR.value("std_out_", "oops");
            string ram_ = jmessageR.value("ram_", "oops");
            string log_ = jmessageR.value("log_", "oops");
            Compiler::updateStrings(std_out_, log_, ram_);
        }
    }

//	Close the socket
close(sock);

return 0;
}

json parseJson (QStringList message, string operation){
    json mymessage =
            {
                    {"type", message.at(0).toStdString()},
                    {"name",message.at(1).toStdString()},
                    {"value",message.at(2).toStdString()},
                    {"key",message.at(3).toStdString()},
                    {"operation", operation},
            };

    // Access the values
    string type = mymessage.value("type", "oops");
    string name = mymessage.value("name", "oops");
    string value = mymessage.value("value", "oops");

    // Print the values
    std::cout << "Type: " << type << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Value: " << value << std::endl;

    return mymessage;

}

QStringList Compiler::Divide(QStringList initial) {
    QString str = initial.join(" ");
    QStringList newList = str.split("=");
    QStringList aux;
    string name = str.toStdString();
    int canti = newList.length();
    switch (canti) {
        case 1:
            aux = newList.at(0).split(" ", QString::SkipEmptyParts);
            if (validename(newList.at(0).toStdString()) && aux.length() == 1) {
                cout  <<"nombre correcto\n";
                string name = newList.at(0).toStdString();
                if (name[name.length()-1] == ';'){
                    newList.replaceInStrings(QRegExp(";"),"");
                }else{cout  <<"declaracion invalida, falta ;\n";}}
            else{ cout<<"Error de nombre de variable, sintaxis incorrecta\n";
                QStringList temp;
                newList = temp;}
            break;
        case 2:
            if (validename(newList.at(0).toStdString())) {
                cout  <<"nombre correcto\n";
                string name = newList.at(1).toStdString();
                aux = newList.at(1).split(" ", QString::SkipEmptyParts);
                if (name[name.length()-1] == ';' && aux.length() == 1){
                    newList.replaceInStrings(QRegExp(";"),"");
                }else{cout  <<"declaracion invalida, falta ;\n";}}
            else{
                cout<<"Error en valor de variable\n";
                QStringList temp;
                newList = temp;
            }
            break;
        default:
            cout<<"Error de sintaxis\n";
            QStringList temp;
            newList = temp;
            break;

    }
    return newList;
}




void Compiler::compile(QString line) {
    QStringList words = line.split(" ");
    if ("int" == words.at(0).toStdString()){
        cout <<"soy un int\n";
        words.removeOne("int");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: {
                newList.prepend("int");
                newList.append(NULL);
                newList.append("define");
                cout <<"Generando variable\n";
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
                }
            case 2:{
                newList.prepend("int");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                    if(newList.at(2).split(separator).length() != 1){
                        cout  <<"Operation"<< endl ;
                        newList.append("define");
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                        cout  <<"get value"<< endl ;
                        newList.append("define");
                        json mymessage = parseJson(newList, "reference");
                        startClient (mymessage);
                    }
                    else{
                        try{
                            if (value.length() == to_string(stoi(value)).length()){
                                newList.append("define");
                                json mymessage = parseJson(newList, "false");
                                startClient (mymessage);
                                break;
                            }else{ cout  <<"tipo no coincide con valor\n";}
                        }catch (std::invalid_argument){
                            newList.append("define");
                            json mymessage = parseJson(newList, "true");
                            startClient (mymessage);
                            break;
                        }
                        break;
                    }
                }
        }
    }

    else if("long" == words.at(0).toStdString()){
        cout <<"soy un long\n";
        words.removeOne("long");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: {
                newList.prepend("long");
                newList.append(NULL);
                newList.append("define");
                cout <<"Generando variable\n";
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{
                newList.prepend("long");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                if(newList.at(2).split(separator).length() != 1){
                    cout  <<"Operation"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient (mymessage);
                    break;
                }
                else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    cout  <<"get value"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                else{
                    try{
                        if (value.length() == to_string(stol(value)).length()){
                            newList.append("define");
                            json mymessage = parseJson(newList, "false");
                            startClient (mymessage);
                            break;
                        }else{ cout  <<"tipo no coincide con valor\n";}
                    }catch (std::invalid_argument){
                        newList.append("define");
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    break;
                }
            }
        }
    }

    else if("char" == words.at(0).toStdString()){
        cout <<"soy un char\n";
        words.removeOne("char");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: {
                //char a;
                newList.prepend("char");
                newList.append(NULL);
                newList.append("define");
                cout <<"Generando variable\n";
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{
                newList.prepend("char");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                /*if(newList.at(2).split(separator).length() != 1){
                    cout  <<"Operation"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient (mymessage);
                    break;
                }*/
                //char a=b.getValue();
                if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    cout  <<"get value"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                else{
                    try{
                        char * ArrayChar = new char[value.length()];
                        for(int i; i<value.length(); i++){
                           ArrayChar[i] = value[i];
                        }
                        //char a='a';
                        if (string(1,value[0]) == "'" && string(1,value[0]) == "'" && value.length() == 3){
                            cout<<"Definido\n";
                            newList.append("define");
                            json mymessage = parseJson(newList, "false");
                            startClient (mymessage);
                            break;
                        }else{
                            newList.append("define");
                            json mymessage = parseJson(newList, "true");
                            startClient (mymessage);
                            break;
                            cout  <<"tipo no coincide con valor\n";}
                    }catch (std::invalid_argument){
                        cout<<"Error en algo del char";
                        break;
                    }
                    break;
                }
            }
        }
    }

    else if("float" == words.at(0).toStdString()){
        cout <<"soy un foat\n";
        words.removeOne("float");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: {
                newList.prepend("float");
                newList.append(NULL);
                newList.append("define");
                cout <<"Generando variable\n";
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{
                newList.prepend("float");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                if(newList.at(2).split(separator).length() != 1){
                    cout  <<"Operation"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient (mymessage);
                    break;
                }
                else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    cout  <<"get value"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                else{
                    try{
                        if (newList.at(2).split(".").length() == 2) {
                            string entero = newList.at(2).split(".").at(0).toStdString();
                            string decimal = newList.at(2).split(".").at(1).toStdString();
                            if (entero.length() == to_string(stol(entero)).length()
                                && decimal.length() == to_string(stol(decimal)).length()) {

                                newList.append("define");
                                json mymessage = parseJson(newList, "false");
                                startClient(mymessage);
                                break;
                            } else { cout << "tipo no coincide con valor\n"; }
                        }else {
                            cout<< "Valor invalido\n";}

                    }catch (std::invalid_argument){
                        newList.append("define");
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    break;
                }
            }
        }
    }

    else if("double" == words.at(0).toStdString()){
        cout <<"soy un double\n";
        words.removeOne("double");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: {
                newList.prepend("double");
                newList.append(NULL);
                newList.append("define");
                cout <<"Generando variable\n";
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{
                newList.prepend("double");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                if(newList.at(2).split(separator).length() != 1){
                    cout  <<"Operation"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient (mymessage);
                    break;
                }
                else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    cout  <<"get value"<< endl ;
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                else{
                    try{
                        if (newList.at(2).split(".").length() == 2) {
                            string entero = newList.at(2).split(".").at(0).toStdString();
                            string decimal = newList.at(2).split(".").at(1).toStdString();
                            if (entero.length() == to_string(stol(entero)).length()
                                && decimal.length() == to_string(stol(decimal)).length()) {

                                newList.append("define");
                                json mymessage = parseJson(newList, "false");
                                startClient(mymessage);
                                break;
                            } else { cout << "tipo no coincide con valor\n"; }
                        }else {
                            cout<< "Valor invalido\n";}
                    }catch (std::invalid_argument){
                        newList.append("define");
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    break;
                }
            }
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
    }**/

    else if("reference" == words.at(0).toStdString()){
        words.removeFirst();
        string type = valideReference(words.at(0).toStdString());
        if (type != "false") {
            words.removeFirst();
            QStringList newList = Divide(words);
            int size = newList.length();
            cout<<"Size: "<<size<<endl;
            switch (size) {
                case 1: {
                    newList.prepend(QString::fromStdString(type));
                    newList.append(NULL);
                    newList.append("defineR");
                    cout <<"Generando reference\n";
                    json mymessage = parseJson(newList, "false");
                    startClient(mymessage);
                    break;
                }
                case 2:{
                    newList.prepend(QString::fromStdString(type));
                    string value = newList.at(2).toStdString();

                    if(newList.at(2).split(".").length() == 1){
                        cout<<"no hay get\n";
                        newList.append("defineR");
                        json mymessage = parseJson(newList, "false");
                        startClient (mymessage);
                        break;
                    }
                    else if(newList.at(2).split(".").at(1).toStdString() == "getAddr()"){
                        cout<<"si hay get\n";
                        newList.append("defineR");
                        //value.erase(value.end()-10, value.end());
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }else{ cout<< "Sintaxis Error\n";}
                }
            }
            cout << "nombre correcto\n";

        }
        else{
            cout  <<"Tipo incorrecto\n";
        }

    }

    else if("print" == words.at(0).toStdString()){
        if (words.length() == 2){
            words.removeFirst();
            words.replaceInStrings(QRegExp(";"),"");
            words.prepend(QString::fromStdString("print"));
            words.prepend(QString::fromStdString("print"));
            words.append("print");
            json mymessage = parseJson(words, "true");
            startClient (mymessage);
        }
        else{
            cout <<"error sintatactico en la funcion print\n";
        }

    }
    else if("cancel" == words.at(0).toStdString()){
        words.prepend(QString::fromStdString("cancel"));
        words.prepend(QString::fromStdString("cancel"));
        words.prepend(QString::fromStdString("cancel"));
        json mymessage = parseJson(words, "true");
        startClient (mymessage);
    }
    else{
        cout <<"error\n";
    }
}

string Compiler::valideReference(std::string name) {
    if(name == "<int>" || name == "<long>" || name == "<char>" || name == "<float>" || name == "<double>"){
        name.erase(name.begin());
        int size = name.size();
        name.erase(name.begin()+(size-1));
        return name;
    } else {return "false";}
}


bool Compiler::validename(string name) {
    cout << name <<"\n";
    if (name != "int" && name != "long" && name != "char" && name != "float" && name != "double" && name != "struct" &&
        name != "reference") {
        if ((name[0] == '_') || (isalpha(name[0]))) {
            for (int i = 1; i < name.length(); i++) {
                if (isalpha(name[i]) || name[i] == '_' || isdigit(name[i])) {
                    continue;
                } else if (i + 1 == name.length() && name[i] == ';') {
                    return true;
                } else { return false; }
            }
            return true;
        } else { return false; }
    } else { return false; }
}




void Compiler::sendServer() {}

void Compiler::updateStrings(string stdout_, string log_, string ram_) {
    Compiler::std_out += stdout_;
    Compiler::log += log_;
    Compiler::ram = ram_;
}

string Compiler::updateGUI() {
    string package = Compiler::std_out+ "-" + Compiler::log + "-" + Compiler::ram;
    return package;
}






