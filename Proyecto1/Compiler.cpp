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
    int port = 54000;
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
    cout << mymessage;
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
        //		Display response
        cout << "SERVER> " << string(buf, bytesReceived) << "\r\n";
    }

//	Close the socket
close(sock);

return 0;
}

json parseJson (QStringList message){
    // jdEmployees
    json mymessage =
            {
                    {"type", message.at(0).toStdString()},
                    {"name",message.at(1).toStdString()},
                    {"value",message.at(2).toStdString()},
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
                    newList.prepend("int");
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
                    newList.prepend("int");
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
            case 2: {
                newList.append(NULL);
                cout <<"Generando variable\n";
                json mymessage = parseJson(newList);
                thread th (startClient, mymessage);
                th.detach();
                break;
                }
            case 3:{
                string value = newList.at(2).toStdString();
                try{
                    if (value.length() == to_string(stoi(value)).length()){
                        json mymessage = parseJson(newList);
                        //thread th (startClient, mymessage);
                        //th.detach();
                    }else{ cout  <<"tipo no coincide con valor\n";}
                }catch (std::invalid_argument){
                    cout  <<"tipo no coincide con valor\n";
                }
                break;
                }
        }


        /**
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
        }**/
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



bool Compiler::validename(string name) {
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

void Compiler::updateStrings(string std_out, string log, string ram) {
    this->std_out = std_out;
    this->log = log;
    this->ram = ram;
}

string Compiler::updateGUI() {
    string package = this->std_out+ "-" + this->log + "-" + this->ram;
    return package;
}






