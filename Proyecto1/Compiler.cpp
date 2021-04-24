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
#include <chrono>
#include <ctime>

using namespace std;
using json = nlohmann::json;

/**
 * bool in_scope: Define si el codigo ingresado esta en un Scope
 * bool in_struct: Define si el codigo ingresado esta en un Struct
 * int deap: Lleva un conteo cuantos Scopes estan abiertos
 */
bool in_scope = false;
bool in_struct=false;
int deap= 0;

/**
 * Funcion para agregar mensaje al Logger
 * @param message
 */
void addTolog (string message){
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    string date = ctime(&now_time);
    date.erase(date.end() - 1, date.end());
    Compiler::log += date + " " + message + "\n";
}

/**
 * Funcion para Iniciar el cliente del IDE y establecer la conexion con el mServer
 * @param message Es un Json de linea de codigo que se desea ejecutar
 * @return
 */
int startClient(json message) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1){
        return 1;
    }

    int port = Compiler::port;    //Asignacion de IP y puerto para la conexion
    string ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));  //Conexion con el mServer
    if (connectRes == -1){
        addTolog("ERROR: No se puede conectar con el mServer!");
        return 1;
    }

    char buf[4096];
    string userInput;
    string mymessage = message.dump();
    addTolog("INFO: Conectando.. Enviando linea de codigo");

    int sendRes = send(sock, mymessage.c_str(), mymessage.size() + 1, 0);  //Envio al mServer
    if (sendRes == -1){
        addTolog("ERROR: No se pudo enviar el mensaje al mServer!");
    }

    //		Wait for response
    memset(buf, 0, 4096);
    int bytesReceived = recv(sock, buf, 4096, 0);
    if (bytesReceived == -1){
        addTolog("ERROR: Ha ocurrido un error recibiendo el mensaje del mServer");
    }
    else{
        string messageR = string(buf, bytesReceived);
        if(messageR != "") {
            json jmessageR = json::parse(messageR);
            string destroyed = jmessageR.value("destroyed", "oops");
            if (destroyed != ""){
                QString Qdestroyed = QString::fromStdString(destroyed);
                QStringList deleted = Qdestroyed.split("$");
                for (int i = 0; i < deleted.size(); i++) {
                    Compiler::s_existing.erase(std::remove( Compiler::s_existing.begin(),  Compiler::s_existing.end()
                                                            , deleted.at(i).toStdString()),  Compiler::s_existing.end());
                }
            }
            string std_out_ = jmessageR.value("std_out_", "oops");
            string ram_ = jmessageR.value("ram_", "oops");
            string log_ = jmessageR.value("log_", "oops");
            Compiler::updateStrings(std_out_, log_, ram_);
            addTolog("INFO: Mensaje recibido del mServer.. Actualizando..");
        }
    }

close(sock); //Cierra el socket de escucha

return 0;
}
/**
 * Funcion que convierte el mensjae que se desea enviar a Json para su serializacion
 * @param message QStringList que contiene las partes del mensaje
 * @param operation Un key para definir ciertas funciones
 * @return Json serializado
 */
json parseJson (QStringList message, string operation){
    json mymessage =
            {
                    {"type", message.at(0).toStdString()},
                    {"name",message.at(1).toStdString()},
                    {"value",message.at(2).toStdString()},
                    {"key",message.at(3).toStdString()},
                    {"operation", operation},
            };
    addTolog("INFO: Serializando linea de codigo a Json");
    return mymessage;
}
/**
 * Funcion que determina la linea de codigo que se analiza, define si se debe
 * de hacer funciones o simples creaciones
 * @param initial Linea de codigo que se analiza
 * @return Parametros para funciones en el mServer
 */
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
                string name = newList.at(0).toStdString();
                if (name[name.length()-1] == ';'){
                    newList.replaceInStrings(QRegExp(";"),"");
                }else{
                    addTolog("ERROR: Declaracion invalida");
                    QStringList temp;
                    newList = temp;
                }}
            else{ addTolog("ERROR: Problema en nombre de variable, sintaxis incorrecta");
                QStringList temp;
                newList = temp;}
            break;
        case 2:
            if (validename(newList.at(0).toStdString())) {
                string name = newList.at(1).toStdString();
                aux = newList.at(1).split(" ", QString::SkipEmptyParts);
                if (name[name.length()-1] == ';' && aux.length() == 1){
                    newList.replaceInStrings(QRegExp(";"),"");
                }else{
                    addTolog("ERROR: Declaracion invalida");
                    QStringList temp;
                    newList = temp;
                }}
            else{
                addTolog("ERROR: Problema en variable");
                QStringList temp;
                newList = temp;
            }
            break;
        default:
            addTolog("ERROR: Problema de sintaxis");
            QStringList temp;
            newList = temp;
            break;

    }
    return newList;
}

/**
 * Define los parametros necesarios para el procesamiento en el mServer
 * @param line Linea compilada en el IDE
 */
void Compiler::compile(QString line) {
    QStringList words = line.split(" ");
    if ("int" == words.at(0).toStdString()){
        words.removeOne("int");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: { //Caso en el que solo se posee una variable
                newList.prepend("int");
                newList.append(NULL);
                newList.append("define");
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
                }
            case 2:{ //Caso en el que se posee una variable con valor
                newList.prepend("int");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                    if(newList.at(2).split(separator).length() != 1){ //Valor con operaciones aritmeticas
                        newList.append("define");
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    //Valor con el identificador getValue
                    else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                        newList.append("define");
                        json mymessage = parseJson(newList, "reference");
                        startClient (mymessage);
                    }
                    //Valor con una referencia a un Struct
                    else if(newList.at(2).split(".").length() == 2){
                        newList.append("define");
                        json mymessage = parseJson(newList, "struct");
                        startClient (mymessage);
                    }
                    //Valor es un numero o una variable
                    else{
                        try{
                            if (value.length() == to_string(stoi(value)).length()){ //Revisa si es un numero y tipo
                                newList.append("define");
                                json mymessage = parseJson(newList, "false");
                                startClient (mymessage);
                                break;
                            }else{ addTolog("ERROR: Tipo no coincide con valor");}
                        }catch (std::invalid_argument){ //Envia la variable igualada
                            newList.append("define");
                            json mymessage = parseJson(newList, "true");
                            startClient (mymessage);
                            break;
                        }
                        break;
                    }
                }
            default:{
                addTolog("ERROR: Variable no creada");
            }
        }
    }

    else if("long" == words.at(0).toStdString()){
        words.removeOne("long");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: { //Caso en el que solo se posee una variable
                newList.prepend("long");
                newList.append(NULL);
                newList.append("define");
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{ //Caso en el que se posee una variable con valor
                newList.prepend("long");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                if(newList.at(2).split(separator).length() != 1){ //Valor con operaciones aritmeticas
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient (mymessage);
                    break;
                }
                    //Valor con el identificador getValue
                else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                    //Valor con una referencia a un Struct
                else if(newList.at(2).split(".").length() == 2){
                    newList.append("define");
                    json mymessage = parseJson(newList, "struct");
                    startClient (mymessage);
                }
                    //Valor es un numero o una variable
                else{
                    try{
                        if (value.length() == to_string(stol(value)).length()){
                            newList.append("define");
                            json mymessage = parseJson(newList, "false");
                            startClient (mymessage);
                            break;
                        }else{addTolog("ERROR: Tipo no coincide con valor");}
                    }catch (std::invalid_argument){
                        newList.append("define");
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    break;
                }
            }
            default:{
                addTolog("ERROR: Variable no creada");
            }
        }
    }

    else if("char" == words.at(0).toStdString()){
        words.removeOne("char");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: { //Caso en el que solo se posee una variable
                //char a;
                newList.prepend("char");
                newList.append(NULL);
                newList.append("define");
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{ //Caso en el que se posee una variable con valor
                newList.prepend("char");
                string value = newList.at(2).toStdString();
                //Si el valor posee el identificador getValue
                if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                //Si el valor es de tipo char
                else{
                    char * ArrayChar = new char[value.length()];
                    for(int i; i<value.length(); i++){
                       ArrayChar[i] = value[i];
                    }
                    if (string(1,value[0]) == "'" && string(1,value[2]) == "'" && value.length() == 3){
                        newList.append("define");
                        json mymessage = parseJson(newList, "false");
                        startClient (mymessage);

                    }else{
                        if(newList.at(2).split(".").length() == 2){
                            newList.append("define");
                            json mymessage = parseJson(newList, "struct");
                            startClient (mymessage);
                        }else{
                            newList.append("define");
                            json mymessage = parseJson(newList, "true");
                            startClient (mymessage);
                        }
                    }
                }
            }
            default:{
                addTolog("ERROR: Variable no creada");
            }
        }
    }

    else if("float" == words.at(0).toStdString()){
        words.removeOne("float");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: { //Caso en el que solo se posee una variable
                newList.prepend("float");
                newList.append(NULL);
                newList.append("define");
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{ //Caso en el que se posee una variable con valor
                newList.prepend("float");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                if(newList.at(2).split(separator).length() != 1){ //Valor con operaciones aritmeticas
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient (mymessage);
                    break;
                }
                    //Valor con el identificador getValue
                else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                    //Valor es un numero, una variable o ref a Struct
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
                            } else { addTolog("ERROR: Tipo no coincide con valor"); }
                        }else {
                            addTolog("ERROR: Valor invalido");}

                    }catch (std::invalid_argument){
                        //Valor con una referencia a un Struct
                        if(newList.at(2).split(".").length() == 2){
                            newList.append("define");
                            json mymessage = parseJson(newList, "struct");
                            startClient (mymessage);
                        }else{
                            newList.append("define");
                            json mymessage = parseJson(newList, "true");
                            startClient (mymessage);
                        }
                        break;

                    }
                    break;
                }
            }
            default:{
                addTolog("ERROR: Variable no creada");
            }
        }
    }

    else if("double" == words.at(0).toStdString()){
        words.removeOne("double");
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: { //Caso en el que solo se posee una variable
                newList.prepend("double");
                newList.append(NULL);
                newList.append("define");
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2:{ //Caso en el que se posee una variable con valor
                newList.prepend("double");
                string value = newList.at(2).toStdString();
                QRegExp separator("([-+*/])");
                if(newList.at(2).split(separator).length() != 1){ //Valor con operaciones aritmeticas
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient (mymessage);
                    break;
                }
                    //Valor con el identificador getValue
                else if(newList.at(2).split(".").length() == 2 && newList.at(2).split(".").at(1).toStdString() == "getValue()"){
                    newList.append("define");
                    json mymessage = parseJson(newList, "reference");
                    startClient (mymessage);
                }
                    //Valor con una referencia a un Struct
                else if(newList.at(2).split(".").length() == 2){
                    newList.append("define");
                    json mymessage = parseJson(newList, "struct");
                    startClient (mymessage);
                }
                    //Valor es un numero o una variable
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
                            } else { addTolog("ERROR: Tipo no coincide con valor"); }
                        }else {
                            addTolog("ERROR: Valor invalido");}
                    }catch (std::invalid_argument){
                        newList.append("define");
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    break;
                }
            }
            default:{
                addTolog("ERROR: Variable no creada");
            }
        }
    }

    else if(words.at(0).toStdString() == "struct" && string(1,words.at(1).toStdString()[words.at(1).toStdString().length()-1]) == "{"  && !in_struct){
        string name = words.at(1).toStdString();
        name.erase(name.end()-1);
        if (validename(name)){ //Valida el nombre
            Compiler::s_existing.push_back(name);
            in_struct = true;
            words.append(QString::fromStdString("struct"));
            words.append(QString::fromStdString("struct"));
            json mymessage = parseJson(words, "false");
            startClient (mymessage);
        }
    }
    //Valida si se esta cerrando el struct
    else if(words.at(0).toStdString() == "};" && in_struct){
        in_struct = false;
        words.prepend(QString::fromStdString("struct"));
        words.prepend(QString::fromStdString("struct"));
        words.append(QString::fromStdString("struct"));
        json mymessage = parseJson(words, "false");
        startClient (mymessage);
    }
    //Valida si se crea una referencia
    else if("reference" == words.at(0).toStdString()){
        words.removeFirst();
        string type = valideReference(words.at(0).toStdString()); //Valida si la definicion de tipo es correcto
        if (type != "false") {
            words.removeFirst();
            QStringList newList = Divide(words);
            int size = newList.length();
            switch (size) {
                case 1: { //Caso en el que solo se posee una variable
                    newList.prepend(QString::fromStdString(type));
                    newList.append(NULL);
                    newList.append("defineR");
                    json mymessage = parseJson(newList, "false");
                    startClient(mymessage);
                    break;
                }
                case 2:{ //Caso en que posee una igualacion
                    newList.prepend(QString::fromStdString(type));
                    string value = newList.at(2).toStdString();

                    if(newList.at(2).split(".").length() == 1){ //Si esta igualado a otra referencia
                        newList.append("defineR");
                        json mymessage = parseJson(newList, "false");
                        startClient (mymessage);
                        break;
                    }
                        //Valor con el identificador getAddr
                    else if(newList.at(2).split(".").at(1).toStdString() == "getAddr()"){
                        newList.append("defineR");
                        //value.erase(value.end()-10, value.end());
                        json mymessage = parseJson(newList, "true");
                        startClient (mymessage);
                        break;
                    }
                    //Si esta igualado a una referencia de un struct
                    else if(newList.at(2).split(".").length() == 2){
                        newList.append("define");
                        json mymessage = parseJson(newList, "struct");
                        startClient (mymessage);
                    }
                    else{ addTolog("ERROR: Sintaxis Error");}
                }
                default:{
                    addTolog("ERROR: Variable no creada");
                }
            }
        }
        else{
            addTolog("ERROR: Tipo incorrecto");
        }

    }

    else if("print" == words.at(0).toStdString()){ //Detecta si se debe de imprimir en STD_OUT
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
            addTolog("ERROR: Problema sintatactico en la funcion print");
        }

    }

    else if("y!@#$%^&*(" == words.at(0).toStdString()){ //Palabra clave para eliminar
        if (in_struct || in_scope){
            addTolog("WARN: Scope o Struct abierto");
        }
        words.append(QString::fromStdString("cancel"));
        words.append(QString::fromStdString("cancel"));
        words.append(QString::fromStdString("cancel"));
        in_scope = false;
        in_struct = false;
        deap = 0;
        json mymessage = parseJson(words, "true");
        startClient (mymessage);
    }

    //Valida si se esta abriendo un scope
    else if("{" == words.at(0).toStdString() && words.replaceInStrings("{","").at(0).toStdString() == "" ){
        if(!in_struct) {
            in_scope = true;
            deap++;
            words.append(QString::fromStdString("scope_o"));
            words.append(QString::fromStdString("scope"));
            words.append(QString::fromStdString("scope"));
            json mymessage = parseJson(words, "true");
            startClient(mymessage);
        }
        else{
            addTolog("ERROR: No se permite la anidacion de Struct");
        }
    }
    //Valida si se esta cerrando un scope
    else if("}" == words.at(0).toStdString() && words.replaceInStrings("}","").at(0) == "" && in_scope){
        deap--;
        if (deap ==0){
            in_scope = false;
        }
        words.append(QString::fromStdString("scope_c"));
        words.append(QString::fromStdString("scope"));
        words.append(QString::fromStdString("scope"));
        json mymessage = parseJson(words, "true");
        startClient (mymessage);
    }
    //Valida si el tipo es un tag hacia un Struct
    else if(validenamestruct(words.at(0).toStdString())){
        string type = words.at(0).toStdString();
        words.removeOne(words.at(0));
        QStringList newList = Divide(words);
        int num = newList.length();
        switch (num) {
            case 1: { //Solo se define el struct
                newList.prepend(QString::fromStdString(type));
                newList.append(NULL);
                newList.append("define");
                json mymessage = parseJson(newList, "false");
                startClient(mymessage);
                break;
            }
            case 2: { //Cuando posee una igualacion
                newList.prepend(QString::fromStdString(type));
                if (newList.at(2).split(".").length() == 2) { //Hace referencia a un valor dentro de un Struct
                    newList.append("define");
                    json mymessage = parseJson(newList, "struct");
                    startClient(mymessage);
                } else { //Cuando esta igualado a cualquier otra cosa
                    string value = newList.at(2).toStdString();
                    newList.append("define");
                    json mymessage = parseJson(newList, "true");
                    startClient(mymessage);
                }
                break;
            }
            default: {
                addTolog("ERROR: Variable no creada");
                }
            }
        }
    else{
        if (!in_struct){ //si no esta dentro de un Struct
            QStringList newList = Divide(words);
            int num = newList.length();
            if(num == 2) { //Si posee una igualacion
                QRegExp separator("([-+*/])");
                if (newList.at(1).split(separator).length() == 1) { //SI ES SOLO UNA VARIABLE IGUALADA O REFERENCE
                    if (newList.at(1).split(".").length() == 2
                        && (newList.at(1).split(".").at(1).toStdString() == "getAddr()"
                            || newList.at(1).split(".").at(1).toStdString() == "getValue()")){ //VARIABLE IGUALADA A VARIABLE
                        newList.prepend("reference"); //type
                    }
                    else{
                        newList.prepend("variable"); //type
                    }
                    newList.append("equal");
                    json mymessage = parseJson(newList, "false");
                    startClient(mymessage);
                }
                else{
                    newList.prepend("operation"); //type
                    newList.append("equal"); //key
                    json mymessage = parseJson(newList, "false");
                    startClient(mymessage);
                }
            }else{
                addTolog("ERROR: Operacion invalida");
            }
        }
        else{
            addTolog("ERROR: Operacion invalida dentro del Struct");
        }
    }
}

/**
 * Valida si la definicion del tipo del reference es correcta
 * @param name Tipo del reference
 * @return El tipo correcto o si es falso
 */
string Compiler::valideReference(std::string name) {
    if(name == "<int>" || name == "<long>" || name == "<char>" || name == "<float>" || name == "<double>"){
        name.erase(name.begin());
        int size = name.size();
        name.erase(name.begin()+(size-1));
        return name;
    } else {return "false";}
}

/**
 * Valida el nombre de las variables a crear
 * @param name Nombre de la variable
 * @return bool si es valido
**/
bool Compiler::validename(string name) {
    if (name != "int" && name != "long" && name != "char" && name != "float" && name != "double" && name != "struct" &&
        name != "reference" && !validenamestruct(name)) {
        if ((name[0] == '_') || (isalpha(name[0]))) {
            for (int i = 1; i < name.length(); i++) {
                if (isalpha(name[i]) || name[i] == '_' ||(name[i] == '.') || isdigit(name[i])) {
                    continue;
                } else if (i + 1 == name.length() && name[i] == ';') {
                    return true;
                } else { return false; }
            }
            return true;
        } else { return false; }
    } else { return false; }
}

/**
 * Funcion que valida si el tipo es igual a un tag de un Struct
 * @param name Nombre del struct
 * @return
 */
bool Compiler::validenamestruct(string name) {
    bool flag = false;
    for (int i=0; i< Compiler::s_existing.size(); i++){
        if (name == Compiler::s_existing.at(i)){
            flag=true;
            return flag;
        }
    }
    return flag;
}

/**
 * Funcion para actualizar los strings que se imprimen en el IDE
 * @param stdout_
 * @param log_
 * @param ram_
 */
void Compiler::updateStrings(string stdout_, string log_, string ram_) {
    Compiler::std_out += stdout_;
    Compiler::log += log_;
    if (ram_!=""){
        Compiler::ram = ram_;
    }
}

/**
 * Empaqueta la infromacion nueva para el GUI
 * @return
 */
string Compiler::updateGUI() {
    string package = Compiler::std_out+ "-" + Compiler::log + "-" + Compiler::ram;
    return package;
}






