//
// Created by usuario on 3/28/21.
//
#include <iostream>
#include <thread>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include "CMemoryPool.h"
#include "json.hpp"
#include "Variable.h"
#include <list>
#include "Split_getline.h"
#include <sstream>
#include <QString>
#include <QStringList>
#include <chrono>
#include <ctime>


using json = nlohmann::json;
using namespace std;

bool in_scope = false;                  //Bool de si esta en un Scope
bool in_struct = false;                 //Bool de si esta en un Struct
int deap= 0;                            //Contador de la profundidad en un Scope
vector<string> s_existing;              //Vector de los nombres reservados por Struct
vector<Variable> globalList;            //Vector de referencia a las variables creadas
MemPool::SMemoryChunk *active_struct;   //Guarda el Struct en creacion
string ram_;                            //String que almacena la ram para enviarla a C!
string std_out_;                        //String que almacena el std_out para enviarla a C!
string log_;                            //String que almacena el log para enviarla a C!
string destroyed;                       //Guarda los tags de los Struct a reciclar


/**
 * Funcion para agregar mensaje al Logger
 * @param message
 */
void addTolog (string message){
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    string date = ctime(&now_time);
    date.erase(date.end() - 1, date.end());
    log_ += date + " " + message + "\n";
}


/**
 * Funcion que valida si el tipo es igual a un tag de un Struct
 * @param name Tipo a validar
 * @return
 */
bool validenamestruct(string name) {
    bool flag = false;
    for (int i=0; i< s_existing.size(); i++){
        if (name == s_existing.at(i)){
            flag=true;
            return flag;
        }
    }
    return flag;
}

/**
 * Funcion que convierte el mensjae que se desea enviar a Json para su serializacion
 * @return Json serealizado
 */
json parseJson (){
    json mymessage =
            {
                    {"std_out_", std_out_},
                    {"ram_", ram_},
                    {"log_", log_},
                    {"destroyed", destroyed },

            };
    return mymessage;

}

/**
 * Funcion que actualiza la RAM por medio del Vector de variables
 */
void LtoS(){
    ram_ = "";
    for (int i=0; i<globalList.size(); i++){
        Variable variable = globalList.at(i);
        ostringstream get_the_address;
        get_the_address << variable.ptr;
        string address = get_the_address.str();
        string name = variable.name;
        string value;
        string ref;
        if (variable.ptr->type == "int" && !variable.ptr->isReference){
            value = to_string(*(int*)variable.ptr->Data);
            ref = to_string(variable.ptr->counter);
        }
        else if (variable.ptr->type == "long" && !variable.ptr->isReference){
            value = to_string(*(long*)variable.ptr->Data);
            ref = to_string(variable.ptr->counter);
        }
        else if (variable.ptr->type == "float" && !variable.ptr->isReference){
            value = to_string(*(float*)variable.ptr->Data);
            ref = to_string(variable.ptr->counter);
        }
        else if (variable.ptr->type == "double" && !variable.ptr->isReference){
            value = to_string(*(double*)variable.ptr->Data);
            ref = to_string(variable.ptr->counter);
        }
        else if (variable.ptr->type == "char" && !variable.ptr->isReference){
            value = string(1, *variable.ptr->Data);
            ref = to_string(variable.ptr->counter);
        }
        else if (variable.ptr->type == "struct"){
            value = "Null";
            ref = to_string(variable.ptr->counter);
        }
        else if (variable.ptr->isReference){
            ostringstream get_the_address_r;
            get_the_address_r << variable.ptr->reference;
            value = get_the_address_r.str();
            ref = "Null";
        }

        ram_ +=" "+address+" / "+name+" / "+value+" / "+ref+" \n";

    }
}

/**
 * Funcion que inicializa el mServer para la recepcion de solicitudes
 * @param port Puerto a inicializar
 * @param ptr_mpool Pool de memoria a utilizar
 * @return
 */
int startServer(int port, MemPool::CMemoryPool *ptr_mpool) {
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1)
    {
        cerr << "No se pudo crear el Socket. Cerrando..." << endl;
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);

    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
    bind(listening, (sockaddr*)&hint, sizeof(hint));

    listen(listening, SOMAXCONN);
    while (true) {
        bool isupdate = true;

        sockaddr_in client;
        socklen_t clientSize = sizeof(client);

        int clientSocket = accept(listening, (sockaddr *) &client, &clientSize);

        char host[NI_MAXHOST];      // Nombre del cliente remoto
        char service[NI_MAXSERV];

        memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

        if (getnameinfo((sockaddr *) &client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            cout << host << " conectado " << endl;
        } else {
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            cout << host << " conectado al puerto " << ntohs(client.sin_port) << endl;
        }

        char buf[4096];
        memset(buf, 0, 4096);

        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == -1) {
            cerr << "Error al recibir mensaje. Cerrando..." << endl;
            break;
        }

        if (bytesReceived == 0) {
            cout << "Cliente desconectado " << endl;
            break;
        }
        string messageR = string(buf, 0, bytesReceived);
        json jmessageR = json::parse(messageR);
        addTolog("INFO: Linea de codigo recibida");
        string key = jmessageR.value("key", "oops");
        if (key == "define") { //Definicion de variables que no sean Reference
            string name = jmessageR.value("name", "oops");
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (can) { //Si el nombre no esta reservado
                string type = jmessageR.value("type", "oops");
                if (type == "int") { //Creacion de int
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") { //Definicion con cadena de operaciones
                        if ((int *) ptr_mpool->GetMemory(sizeof(int)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            string value = jmessageR.value("value", "oops");
                            double newvalue = split_getline(value, ptr_mpool);
                            if(newvalue >= 0) { //No negativos
                                int *ptrvar = (int *) ptr_mpool->GetMemory(sizeof(int));  //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                int newvaluei = newvalue;
                                *ptrvar = newvaluei;
                                if (in_struct) {
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);
                                addTolog("INFO: Variable creada");
                            }
                            else{
                                addTolog("ERROR: Asignacion no permitida");
                            }
                        }

                    } else if (operation == "reference") { //Definicion con reference
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) { //Concordancias
                                if ((int *) ptr_mpool->GetMemory(sizeof(int)) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    int *ptrvar = (int *) ptr_mpool->GetMemory(
                                            sizeof(int));   //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = 1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    *ptrvar = *(int *) ptrRef->reference->Data;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }

                            } else { addTolog("ERROR: Las variables no coinciden con el tipo"); }
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }

                    } else if(operation == "struct"){ //Definicion con atributo de Struct
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) { //concordancias
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){ //Verificacion de tipos
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){ //Si atributo pertenece a Struct
                                        if ((int *) ptr_mpool->GetMemory(sizeof(int)) == NULL){ //Si el Pool no tiene espacio
                                            addTolog("FATAL: mServer sin memoria");
                                        }
                                        else {
                                            int *ptrvar = (int *) ptr_mpool->GetMemory(
                                                    sizeof(int));   //CREACION DE VARIABLE CON EL POOL CREADO
                                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(
                                                    ptrvar);
                                            ptrChunk->name = name;
                                            ptrChunk->type = type;
                                            ptrChunk->counter = 1;
                                            ptrChunk->isscope = in_scope;
                                            ptrChunk->deap = deap;
                                            *ptrvar = *(int *) ptrvariable->Data;
                                            if (in_struct) {
                                                ptrChunk->isstruct = true;
                                                Variable_struct *variableStruct = new Variable_struct(name);
                                                active_struct->mstruct.push_back(*variableStruct);
                                            }
                                            Variable *variable = new Variable(name,
                                                                              ptrChunk);
                                            globalList.push_back(*variable);
                                            addTolog("INFO: Variable creada");
                                        }
                                    }
                                    else{
                                        addTolog("ERROR: La variable no existe en el Struct");
                                    }
                                }
                               else{
                                    addTolog("ERROR: Tipo de variable accedida no es compatible");
                               }
                            }
                            else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct"); }
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }
                    }

                    else { // Si esta igualado a un numero
                        if ((int *) ptr_mpool->GetMemory(sizeof(int)) == NULL){
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            int *ptrvar = (int *) ptr_mpool->GetMemory(
                                    sizeof(int));  //CREACION DE VARIABLE CON EL POOL CREADO
                            string value = jmessageR.value("value", "oops");
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter = 1;
                            ptrChunk->isscope = in_scope;
                            ptrChunk->deap = deap;
                            if (value != "") {
                                *ptrvar = stoi(value);
                            } else { *ptrvar = NULL; }
                            if (in_struct) {
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            addTolog("INFO: Variable creada");
                        }

                    }
                }

                else if (type == "long") { //Creacion de long
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") { //Definicion con cadena de operaciones
                        if ((long *) ptr_mpool->GetMemory(sizeof(long)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            string value = jmessageR.value("value", "oops");
                            double newvalue = split_getline(value, ptr_mpool);
                            if (newvalue >= 0) { //No negativos
                                long *ptrvar = (long *) ptr_mpool->GetMemory(
                                        sizeof(long));  //CREACION DE VARIABLE CON EL POOL CREADO

                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                long newvaluei = newvalue;
                                *ptrvar = newvaluei;
                                if (in_struct) {
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);
                                addTolog("INFO: Variable creada");
                            }
                            else{
                                addTolog("ERROR: Asignacion no permitida");
                            }
                        }

                    } else if (operation == "reference") { //Definicion con Reference
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) { //Concordancias
                                if ((long *) ptr_mpool->GetMemory(sizeof(long)) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    long *ptrvar = (long *) ptr_mpool->GetMemory(
                                            sizeof(long));   //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = 1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    *ptrvar = *(long *) ptrRef->reference->Data;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }

                            } else { addTolog("ERROR: Las variables no coinciden con el tipo"); }
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }

                    }else if(operation == "struct"){ //Definicion con atributo de Struct
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) { //Concordancias
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){ //Verificacion de tipos
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){ //Si atributo pertenece a Struct
                                        if ((long *) ptr_mpool->GetMemory(sizeof(long)) == NULL){ //Si el Pool no tiene espacio
                                            addTolog("FATAL: mServer sin memoria");
                                        }
                                        else {
                                            long *ptrvar = (long *) ptr_mpool->GetMemory(
                                                    sizeof(long));   //CREACION DE VARIABLE CON EL POOL CREADO
                                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(
                                                    ptrvar);
                                            ptrChunk->name = name;
                                            ptrChunk->type = type;
                                            ptrChunk->counter = 1;
                                            ptrChunk->isscope = in_scope;
                                            ptrChunk->deap = deap;
                                            *ptrvar = *(long *) ptrvariable->Data;
                                            if (in_struct) {
                                                ptrChunk->isstruct = true;
                                                Variable_struct *variableStruct = new Variable_struct(name);
                                                active_struct->mstruct.push_back(*variableStruct);
                                            }
                                            Variable *variable = new Variable(name,
                                                                              ptrChunk);
                                            globalList.push_back(*variable);
                                            addTolog("INFO: Variable creada");
                                        }
                                    }
                                    else{
                                        addTolog("ERROR: La variable no existe en el Struct");
                                    }
                                } else { addTolog("ERROR: Tipo de variable accedida no es compatible"); }

                            } else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct"); }
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }
                    }
                    //CIERRE IF OPERATION
                    else { // Si esta igualado a un numero o a nada
                        if ((long *) ptr_mpool->GetMemory(sizeof(long)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            long *ptrvar = (long *) ptr_mpool->GetMemory(
                                    sizeof(long));  //CREACION DE VARIABLE CON EL POOL CREADO
                            string value = jmessageR.value("value", "oops");
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter = 1;
                            ptrChunk->isscope = in_scope;
                            ptrChunk->deap = deap;
                            if (value != "") {
                                *ptrvar = stol(value);
                            } else { *ptrvar = NULL; }
                            if (in_struct) {
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            addTolog("INFO: Variable creada");
                        }
                    }
                }

                else if (type == "float") { //Creacion de float
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") { //Definicion con cadena de operaciones
                        if ((float *) ptr_mpool->GetMemory(sizeof(float)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            string value = jmessageR.value("value", "oops");
                            double newvalue = split_getline(value, ptr_mpool);
                            if (newvalue >= 0) { //No negativos
                                float *ptrvar = (float *) ptr_mpool->GetMemory(
                                        sizeof(float));  //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;

                                float newvaluei = newvalue;
                                *ptrvar = newvaluei;
                                if (in_struct) {
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);
                                addTolog("INFO: Variable creada");
                            }
                            else{
                                addTolog("ERROR: Asignacion no permitida");
                            }
                        }

                    } else if (operation == "reference") { //Definicion con Reference
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {  //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) { //Concordancias
                                if ((float *) ptr_mpool->GetMemory(sizeof(float)) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    float *ptrvar = (float *) ptr_mpool->GetMemory(
                                            sizeof(float));   //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = 1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    *ptrvar = *(float *) ptrRef->reference->Data;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }

                            } else { addTolog("ERROR: Las variables no coinciden con el tipo"); }
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }

                    }else if(operation == "struct"){ //Definicion con atributo de Struct
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) { //Concordancias
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){ //Verificacion de tipos
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){ //Si atributo pertenece a Struct
                                        if ((float *) ptr_mpool->GetMemory(sizeof(float)) == NULL){ //Si el Pool no tiene espacio
                                            addTolog("FATAL: mServer sin memoria");
                                        }
                                        else {
                                            float *ptrvar = (float *) ptr_mpool->GetMemory(
                                                    sizeof(float));   //CREACION DE VARIABLE CON EL POOL CREADO
                                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(
                                                    ptrvar);
                                            ptrChunk->name = name;
                                            ptrChunk->type = type;
                                            ptrChunk->counter = 1;
                                            ptrChunk->isscope = in_scope;
                                            ptrChunk->deap = deap;
                                            *ptrvar = *(float *) ptrvariable->Data;
                                            if (in_struct) {
                                                ptrChunk->isstruct = true;
                                                Variable_struct *variableStruct = new Variable_struct(name);
                                                active_struct->mstruct.push_back(*variableStruct);
                                            }
                                            Variable *variable = new Variable(name,
                                                                              ptrChunk);
                                            globalList.push_back(*variable);
                                            addTolog("INFO: Variable creada");
                                        }
                                    }
                                    else{
                                        addTolog("ERROR: La variable no existe en el Struct");
                                    }
                                } else {addTolog("ERROR: Tipo de variable accedida no es compatible");}
                            }
                            else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct"); }
                        }
                        else { addTolog("ERROR: Variable no existe");
                        }
                    }//CIERRE IF OPERATION
                    else { // Si esta igualado a un numero o a vacio
                        if ((float *) ptr_mpool->GetMemory(sizeof(float)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            float *ptrvar = (float *) ptr_mpool->GetMemory(
                                    sizeof(float));  //CREACION DE VARIABLE CON EL POOL CREADO
                            string value = jmessageR.value("value", "oops");
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter = 1;
                            ptrChunk->isscope = in_scope;
                            ptrChunk->deap = deap;
                            if (value != "") {
                                *ptrvar = stof(value);
                            } else { *ptrvar = NULL; }
                            if (in_struct) {
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            addTolog("INFO: Variable creada");
                        }
                    }
                }

                else if (type == "double") { //Creacion de double
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") { //Definicion con cadena de operaciones
                        if ((double *) ptr_mpool->GetMemory(sizeof(double)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            string value = jmessageR.value("value", "oops");
                            double newvalue = split_getline(value, ptr_mpool);
                            if (newvalue >= 0) { //No negativos
                                double *ptrvar = (double *) ptr_mpool->GetMemory(
                                        sizeof(double));  //CREACION DE VARIABLE CON EL POOL CREADO

                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                double newvaluei = newvalue;
                                *ptrvar = newvaluei;
                                if (in_struct) {
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);
                                addTolog("INFO: Variable creada");
                            }
                            else{
                                addTolog("ERROR: Asignacion no permitida");
                            }
                        }

                    } else if (operation == "reference") { //Definicion con reference
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) { //Concordancias
                                if ((double *) ptr_mpool->GetMemory(sizeof(double)) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    double *ptrvar = (double *) ptr_mpool->GetMemory(
                                            sizeof(double));   //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = 1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    *ptrvar = *(double *) ptrRef->reference->Data;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }

                            } else { addTolog("ERROR: Las variables no coinciden con el tipo");}
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }

                    }else if(operation == "struct"){ //Definicion con atributo de Struct
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) { //Concordancias
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){ //Verificacion de tipos
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){ //Si atributo pertenece a Struct
                                        if ((double *) ptr_mpool->GetMemory(sizeof(double)) == NULL){
                                            addTolog("FATAL: mServer sin memoria");
                                        }
                                        else {
                                            double *ptrvar = (double *) ptr_mpool->GetMemory(
                                                    sizeof(double));   //CREACION DE VARIABLE CON EL POOL CREADO
                                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(
                                                    ptrvar);
                                            ptrChunk->name = name;
                                            ptrChunk->type = type;
                                            ptrChunk->counter = 1;
                                            ptrChunk->isscope = in_scope;
                                            ptrChunk->deap = deap;
                                            *ptrvar = *(double *) ptrvariable->Data;
                                            if (in_struct) {
                                                ptrChunk->isstruct = true;
                                                Variable_struct *variableStruct = new Variable_struct(name);
                                                active_struct->mstruct.push_back(*variableStruct);
                                            }
                                            Variable *variable = new Variable(name,
                                                                              ptrChunk);
                                            globalList.push_back(*variable);
                                            addTolog("INFO: Variable creada");
                                        }
                                    }
                                    else{
                                        addTolog("ERROR: La variable no existe en el Struct");
                                    }
                                } else { addTolog("ERROR: Tipo de variable accedida no es compatible");}
                            }
                            else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct"); }
                        }
                        else { addTolog("ERROR: Variable no existe");
                        }
                    }//CIERRE IF OPERATION

                    else { // Si esta igualado a un numero o a vacio
                        if ((double *) ptr_mpool->GetMemory(sizeof(double)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            double *ptrvar = (double *) ptr_mpool->GetMemory(
                                    sizeof(double));  //CREACION DE VARIABLE CON EL POOL CREADO
                            string value = jmessageR.value("value", "oops");
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter = 1;
                            ptrChunk->isscope = in_scope;
                            ptrChunk->deap = deap;
                            if (value != "") {
                                *ptrvar = stod(value);
                            } else { *ptrvar = NULL; }
                            if (in_struct) {
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            addTolog("INFO: Variable creada");
                        }

                    }
                }

                else if (type == "char") { //Creacion de char
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") { //Definicion con cadena de operaciones
                        string value = jmessageR.value("value", "oops");
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && !ptrRef->isReference) { //Concordancias
                                if ((char *) ptr_mpool->GetMemory(sizeof(char)) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    char *ptrvar = (char *) ptr_mpool->GetMemory(
                                            sizeof(char));  //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = 1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    *ptrvar = *ptrRef->Data;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }

                            } else{
                                addTolog("ERROR: Las variables no coinciden con el tipo");
                            }
                        }
                        else{
                            addTolog("ERROR: Variable no existe");
                        }

                    } else if (operation == "reference") { //Definicion con reference
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) { //Concordancias
                                if ((char *) ptr_mpool->GetMemory(sizeof(char)) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    char *ptrvar = (char *) ptr_mpool->GetMemory(
                                            sizeof(char));   //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = 1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    *ptrvar = *(char *) ptrRef->reference->Data;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }

                            } else { addTolog("ERROR: Las variables no coinciden con el tipo"); }
                        } else {
                            addTolog("ERROR: Variable no existe");
                            }

                    }else if(operation == "struct"){ //Definicion con atributo de Struct
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) { //Concordancias
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type) { //Verificacion de tipos
                                    bool flag = false;
                                    for (int i = 0; i < ptrstruct->reference->mstruct.size(); i++) {
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp) {
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag) { //Si atributo pertenece a Struct
                                        if ((char *) ptr_mpool->GetMemory(sizeof(char)) == NULL) { //Si el Pool no tiene espacio
                                            addTolog("FATAL: mServer sin memoria");
                                        } else {
                                            char *ptrvar = (char *) ptr_mpool->GetMemory(
                                                    sizeof(char));   //CREACION DE VARIABLE CON EL POOL CREADO
                                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(
                                                    ptrvar);
                                            ptrChunk->name = name;
                                            ptrChunk->type = type;
                                            ptrChunk->counter = 1;
                                            ptrChunk->isscope = in_scope;
                                            ptrChunk->deap = deap;
                                            *ptrvar = *(char *) ptrvariable->Data;
                                            if (in_struct) {
                                                ptrChunk->isstruct = true;
                                                Variable_struct *variableStruct = new Variable_struct(name);
                                                active_struct->mstruct.push_back(*variableStruct);
                                            }
                                            Variable *variable = new Variable(name,
                                                                              ptrChunk);
                                            globalList.push_back(*variable);
                                            addTolog("INFO: Variable creada");
                                        }
                                    }
                                    else {
                                        addTolog("ERROR: La variable no existe en el Struct");
                                    }
                                } else { addTolog("ERROR: Tipo de variable accedida no es compatible"); }
                            } else{ addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct");}
                        }
                        else{
                            addTolog("ERROR: Variable no existe");
                        }
                    }//CIERRE IF OPERATION
                    else { // Si esta igualado a un numero o a vacio
                        if ((char *) ptr_mpool->GetMemory(sizeof(char)) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            char *ptrvar = (char *) ptr_mpool->GetMemory(
                                    sizeof(char));  //CREACION DE VARIABLE CON EL POOL CREADO
                            string value = jmessageR.value("value", "oops");
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter = 1;
                            ptrChunk->isscope = in_scope;
                            ptrChunk->deap = deap;
                            char value1 = value[1];
                            if (value != "") {
                                *ptrvar = value1;
                            } else { *ptrvar = NULL; }
                            if (in_struct) {
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            addTolog("INFO: Variable creada");
                        }

                    }

                }//CIERRE DE IF TYPE

                else if(validenamestruct(type)){ //Valida si la linea es un tag de un Struct
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "false"){ //Cuando no posee igualacion
                        if (ptr_mpool->GetMemory(1) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                            string value = jmessageR.value("value", "oops");
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(type);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter = -1;
                            ptrChunk->isscope = in_scope;
                            ptrChunk->deap = deap;
                            ptrChunk->reference = ptrRef;
                            ptrRef->counter++;
                            ptrChunk->isReference = true;
                            if (in_struct) {
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            addTolog("INFO: Variable creada");
                        }
                    }
                    else if(operation == "true"){ //Cuando esta igualado a algo
                        string value = jmessageR.value("value", "oops");
                        if(!ptr_mpool->FindChunkHoldingSameName(value)){
                            MemPool::SMemoryChunk *ptrRef =ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type){ //Verificacion de tipos
                                if (ptr_mpool->GetMemory(1) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                                    string value = jmessageR.value("value", "oops");
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(type);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = -1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    ptrChunk->reference = ptrRef;
                                    ptrRef->counter++;
                                    ptrChunk->isReference = true;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }
                            }  else { addTolog("ERROR: El tipo no coincide"); }
                        } else{
                            addTolog("ERROR: Variable no existe");
                        }
                    }else if(operation == "struct"){ //Definicion con atributo de Struct
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type)) { //Concordancias
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){ //Verificacion de tipos
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){ //Si atributo pertenece a Struct
                                        if (ptr_mpool->GetMemory(1) == NULL){
                                            addTolog("FATAL: mServer sin memoria");
                                        }
                                        else {
                                            void *ptrvar = ptr_mpool->GetMemory(
                                                    1);   //CREACION DE VARIABLE CON EL POOL CREADO
                                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(
                                                    ptrvar);
                                            ptrChunk->name = name;
                                            ptrChunk->type = type;
                                            ptrChunk->counter = 1;
                                            ptrChunk->isscope = in_scope;
                                            ptrChunk->deap = deap;
                                            ptrChunk->reference = ptrvariable->reference;
                                            ptrChunk->isReference = true;
                                            if (in_struct) {
                                                ptrChunk->isstruct = true;
                                                Variable_struct *variableStruct = new Variable_struct(name);
                                                active_struct->mstruct.push_back(*variableStruct);
                                            }
                                            Variable *variable = new Variable(name,
                                                                              ptrChunk);
                                            globalList.push_back(*variable);
                                            addTolog("INFO: Variable creada");
                                        }
                                    }
                                    else {
                                        addTolog("ERROR: La variable no existe en el Struct");
                                    }
                                } else { addTolog("ERROR: Tipo de variable accedida no es compatible"); }
                            } else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct"); }
                        }
                        else{
                            addTolog("ERROR: Variable no existe");
                        }
                    }
                }
            } else {
                addTolog("ERROR: No se pueden definir dos variables con el mismo nombre");
            }
        } //define


        else if (key == "defineR") { //Definicion de un Reference
            string name = jmessageR.value("name", "oops");
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (can) { //Si el nombre NO esta reservado
                string operation = jmessageR.value("operation", "oops");
                if (operation == "true") { //Si posee un getAddr
                    string value = jmessageR.value("value", "oops");
                    value.erase(value.end() - 10, value.end());
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) { //Si la variable existe
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrOrig = ptr_mpool->FindChunkHoldingNameTo(value);
                        if (ptrOrig->type == type && !ptrOrig->isReference && !ptrOrig->isstruct) { //Concordancias
                            if (ptr_mpool->GetMemory(1) == NULL){ //Si el Pool no tiene espacio
                                addTolog("FATAL: mServer sin memoria");
                            }
                            else {
                                void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->isReference = true;
                                ptrChunk->reference = ptrOrig;
                                ptrChunk->counter = -1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                ptrOrig->counter++;
                                if (in_struct) {
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);
                                addTolog("INFO: Variable creada");
                            }

                        } else {addTolog("ERROR: Las variables no coinciden con el tipo");}
                    } else {
                        addTolog("ERROR: Variable no existe");
                    }

                }else if(operation == "struct"){  //Si esta igualado a una referencia de un struct
                    string value = jmessageR.value("value", "oops");
                    QString Qvalue = QString::fromStdString(value);
                    QStringList parts = Qvalue.split(".");
                    string struct_temp = parts.at(0).toStdString();
                    string variable_temp = parts.at(1).toStdString();
                    bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                    if (!can) { //Si el nombre esta reservado
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                        if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) { //Concordancias
                            MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                            if (ptrvariable->type == type){ //Verificacion de tipos
                                bool flag =false;
                                for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                    if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                        flag = true;
                                        break;
                                    }
                                }
                                if (flag){ //Si atributo pertenece a Struct
                                    if (ptr_mpool->GetMemory(1) == NULL){ //Si el Pool no tiene espacio
                                        addTolog("FATAL: mServer sin memoria");
                                    }
                                    else {
                                        void *ptrvar = ptr_mpool->GetMemory(
                                                1);   //CREACION DE VARIABLE CON EL POOL CREADO
                                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                        ptrChunk->name = name;
                                        ptrChunk->type = type;
                                        ptrChunk->counter = 1;
                                        ptrChunk->isscope = in_scope;
                                        ptrChunk->deap = deap;
                                        ptrChunk->reference = ptrvariable->reference;
                                        ptrChunk->isReference = true;
                                        if (in_struct) {
                                            ptrChunk->isstruct = true;
                                            Variable_struct *variableStruct = new Variable_struct(name);
                                            active_struct->mstruct.push_back(*variableStruct);
                                        }
                                        Variable *variable = new Variable(name,
                                                                          ptrChunk);
                                        globalList.push_back(*variable);
                                        addTolog("INFO: Variable creada");
                                    }
                                }else { addTolog("ERROR: La variable no existe en el Struct");}
                            }else{ addTolog("ERROR: Tipo de variable accedida no es compatible");}
                        } else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct");}
                    } else {
                        addTolog("ERROR: Variable no existe");
                    }
                }
                else { //Cuando posee una definicion
                    string value = jmessageR.value("value", "oops");
                    if (value != "") { //Cuando esta igualado a una variable
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->isReference && ptrRef->type == type) { //Concordancias
                                if (ptr_mpool->GetMemory(1) == NULL){ //Si el Pool no tiene espacio
                                    addTolog("FATAL: mServer sin memoria");
                                }
                                else {
                                    MemPool::SMemoryChunk *ptrOrig = ptrRef->reference;
                                    void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->isReference = true;
                                    ptrChunk->reference = ptrOrig;
                                    ptrChunk->counter = -1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    ptrOrig->counter++;
                                    if (in_struct) {
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);
                                    globalList.push_back(*variable);
                                    addTolog("INFO: Variable creada");
                                }

                            } else { addTolog("ERROR: Las variables no coinciden con el tipo");}
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }
                    } else { //Cuando no esta igualado a nada
                        string type = jmessageR.value("type", "oops");
                        if ( ptr_mpool->GetMemory(1) == NULL){ //Si el Pool no tiene espacio
                            addTolog("FATAL: mServer sin memoria");
                        }
                        else {
                            void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->isReference = true;
                            ptrChunk->counter = -1;
                            ptrChunk->isscope = in_scope;
                            ptrChunk->deap = deap;
                            if (in_struct) {
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            addTolog("INFO: Variable creada");
                        }
                    }
                }
            } else {
                addTolog("ERROR: No se pueden definir dos variables con el mismo nombre");
            }
        }
        else if (key == "print") { //Cuando se desea imprimir una variable
            string value = jmessageR.value("value", "oops");
            bool can = ptr_mpool->FindChunkHoldingSameName(value);
            if (!can) { //Si el nombre esta reservado
                MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                string type = ptrRef->type;
                //Se accede a la variable dependiendo del tipo
                if (type == "int" && !ptrRef->isReference) {
                    int valueprint = *((int *) ptrRef->Data);
                    std_out_ = to_string(valueprint) + "\n";

                } else if (type == "long" && !ptrRef->isReference) {
                    long valueprint = *((long *) ptrRef->Data);
                    std_out_ = to_string(valueprint) + "\n";

                } else if (type == "float" && !ptrRef->isReference) {
                    float valueprint = *((float *) ptrRef->Data);
                    std_out_ = to_string(valueprint) + "\n";

                } else if (type == "double" && !ptrRef->isReference) {
                    double valueprint = *((double *) ptrRef->Data);
                    std_out_ = to_string(valueprint) + "\n";

                } else if (type == "char" && !ptrRef->isReference) {
                    std_out_ = string(1, *ptrRef->Data) + "\n";
                }
                else if (type == "reference") {
                    ostringstream get_the_address_r;
                    get_the_address_r << ptrRef->reference;
                    std_out_ = get_the_address_r.str(); + "\n";
                }
            } else {
                addTolog("ERROR: Variable no existe");
            }

        }
        else if (key == "equal") { //Igualacion de variables
            string name = jmessageR.value("name", "oops");
            QString Qname = QString::fromStdString(name);
            QStringList parts_n = Qname.split(".");
            if (parts_n.length() == 2){ //Si la izquierda es una referencia a una variable dentro de un Struct
                name = parts_n.at(1).toStdString();
            }
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (!can) { //Si el nombre esta reservado
                MemPool::SMemoryChunk *ptrOrg = ptr_mpool->FindChunkHoldingNameTo(name);
                if (!ptrOrg->isReference) { //Si el puntero no es una referencia
                    string type = jmessageR.value("type", "oops");
                    string value = jmessageR.value("value", "oops");
                    if (type == "variable") { //igualado a numero o variable
                        try { //Comprobacion del tipo de la variable
                            if (ptrOrg->type == "char" && string(1, value[0]) == "'" && string(1, value[2]) == "'" &&
                                value.length() == 3) {
                                *ptrOrg->Data = value[1];
                            }
                            else if (ptrOrg->type == "int" && value.length() == to_string(stoi(value)).length()) {
                                *ptrOrg->Data = stoi(value);
                            } else if (ptrOrg->type == "long" && value.length() == to_string(stol(value)).length()) {
                                *ptrOrg->Data = stol(value);
                            } else if (ptrOrg->type == "float" && QString::fromStdString(value).split(".").length() ==2){
                                string entero = QString::fromStdString(value).split(".").at(0).toStdString();
                                string decimal = QString::fromStdString(value).split(".").at(1).toStdString();

                                if(entero.length() == to_string(stol(entero)).length() &&
                                decimal.length() == to_string(stol(decimal)).length()){
                                    *ptrOrg->Data = stof(value);
                                }
                            } else if (ptrOrg->type == "double" && QString::fromStdString(value).split(".").length() ==2) {
                                string entero = QString::fromStdString(value).split(".").at(0).toStdString();
                                string decimal = QString::fromStdString(value).split(".").at(1).toStdString();

                                if (entero.length() == to_string(stol(entero)).length() &&
                                    decimal.length() == to_string(stol(decimal)).length()) {
                                    *ptrOrg->Data = stod(value);
                                }
                            }
                            else {
                                stod("a");
                            }

                        } catch (std::invalid_argument) { //Igualado a un valor que no es aceptado por ningun tipo
                            QString Qvalue = QString::fromStdString(value);
                            QStringList parts_v = Qvalue.split(".");
                            if (parts_v.length() == 2){ //Si la derecha es una referencia a una variable dentro de un Struct
                                string struct_temp = parts_v.at(0).toStdString();
                                string variable_temp = parts_v.at(1).toStdString();
                                bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                                if (!can) { //Si el nombre esta reservado
                                    string type = jmessageR.value("type", "oops");
                                    MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                                    if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) { //Concordancias
                                        MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                        if (ptrvariable->type == ptrOrg->type){ //Verificacion de tipos
                                            bool flag =false;
                                            for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                                if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                                    flag = true;
                                                    break;
                                                }
                                            }
                                            if (flag){ //Si el atributo pertenece a Struct
                                                if (ptrOrg->type != "struct" && ptrOrg->type != "reference") {
                                                    *ptrOrg->Data = *ptrvariable->Data;
                                                }
                                            }else{ addTolog("ERROR: La variable no existe en el Struct");}
                                        } else{addTolog("ERROR: Tipo de variable accedida no es compatible");}
                                    } else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct");}
                                } else {
                                    addTolog("ERROR: Variable no existe");
                                }
                            }
                            else{ //Igualado a una variable
                                bool isVariable = ptr_mpool->FindChunkHoldingSameName(value);
                                if (!isVariable) { //Si el nombre de lo asignado esta reservado
                                    MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                                    if (ptrOrg->type == ptrRef->type) {
                                        *ptrOrg->Data = *ptrRef->Data;

                                    } else { addTolog("ERROR: La variable igualada no coincide con el tipo");}
                                }else{
                                    addTolog("ERROR: Variable igualada no existe");
                                }
                            }

                        }

                    } else if (type == "reference") { //Igualado a una referencia
                        value.erase(value.end() - 11, value.end());
                        bool valide = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!valide) { //Si el nombre de lo asignado esta reservado
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == ptrOrg->type && ptrRef->isReference && !ptrRef->isstruct) {
                                *ptrOrg->Data = *ptrRef->reference->Data;

                            } else {
                                addTolog("ERROR: Operacion no valida");
                            }
                        }else{
                            addTolog("ERROR: Variable igualada no existe");
                        }

                    } else if (type == "operation") { //Si esta igualado a una operacion
                        double newvalue = split_getline(value, ptr_mpool);
                        if (newvalue >= 0) { //No negativos
                            //Comprobacion de tipos
                            if (ptrOrg->type == "int") {
                                int newvaluei = newvalue;
                                *ptrOrg->Data = newvaluei;
                            } else if (ptrOrg->type == "long") {
                                long newvaluei = newvalue;
                                *ptrOrg->Data = newvaluei;
                            } else if (ptrOrg->type == "float") {
                                float newvaluei = newvalue;
                                *ptrOrg->Data = newvaluei;
                            } else if (ptrOrg->type == "double") {
                                double newvaluei = newvalue;
                                *ptrOrg->Data = newvaluei;
                            } else { addTolog("ERROR: El tipo no soporta las operaciones"); }
                        } else {addTolog("ERROR: Asignacion no permitida");}
                    }
                } else if (ptrOrg->isReference && ptrOrg->type !="struct") { // Cuando es una referencia y no es un Struct
                    string type = jmessageR.value("type", "oops");
                    string value = jmessageR.value("value", "oops");
                    if (type == "variable") { //igualado a una variable
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        if (parts.length() == 2){ //Igualado a un atributo de un Struct
                            string struct_temp = parts.at(0).toStdString();
                            string variable_temp = parts.at(1).toStdString();
                            bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                            if (!can) { //Si el nombre de la referencia al Struct esta reservado
                                string type = jmessageR.value("type", "oops");
                                MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                                if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                    MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                    if (ptrvariable->type == ptrOrg->type){ //Verificacion de tipos
                                        bool flag =false;
                                        for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                            if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                                flag = true;
                                                break;
                                            }
                                        }
                                        if (flag){ //Si atributo pertenece a Struct
                                            if (ptrOrg->reference != NULL) { //El puntero original almacenaba una referencia
                                                ptrOrg->reference->counter--;
                                            }
                                            ptrvariable->reference->counter++;
                                            ptrOrg->reference = ptrvariable->reference;
                                        }else {
                                            addTolog("ERROR: La variable no existe en el Struct");
                                        }
                                    } else {addTolog("ERROR: Tipo de variable accedida no es compatible");}
                                } else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct"); }
                            } else {
                                addTolog("ERROR: Variable no existe");
                            }
                        }
                        else{ //Si esta igualado a variable
                            bool isVariable = ptr_mpool->FindChunkHoldingSameName(value);
                            if (!isVariable) { //Si el nombre esta reservado
                                MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                                if (ptrOrg->type == ptrRef->type) { //Verificacion de tipos
                                    if (ptrOrg->reference != NULL) { //El puntero original almacenaba una referencia
                                        ptrOrg->reference->counter--;
                                    }
                                    ptrRef->reference->counter++;
                                    ptrOrg->reference = ptrRef->reference;
                                } else{addTolog("ERROR: La variable no existe en el Struct");}
                            } else { addTolog("ERROR: Variable no existe");}
                        }

                    } else if (type == "reference") { //igualado a una referencia
                        value.erase(value.end() - 10, value.end());
                        bool valide = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!valide) { //Si el nombre esta reservado
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == ptrOrg->type && !ptrRef->isReference && !ptrRef->isstruct) {
                                if (ptrOrg->reference != NULL) { //El puntero original almacenaba una referencia
                                    ptrOrg->reference->counter--;
                                }
                                ptrRef->counter++;
                                ptrOrg->reference = ptrRef;

                            } else {
                                addTolog("ERROR: Operacion no valida");
                            }
                        } else { addTolog("ERROR: Variable no existe"); }
                    }
                    else if(type == "operation"){ //Atributo de Struct es igualado a un valor
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) { //Si el nombre esta reservado
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == ptrOrg->type){ //Verificacion de tipos
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){ //Si atributo pertenece a Struct
                                        if (ptrOrg->type != "struct" && ptrOrg->type != "reference") {
                                            *ptrOrg->Data = *ptrvariable->Data;
                                            ptrvariable->counter++;
                                        }
                                        else if( ptrOrg->type == "reference"){
                                            ptrOrg->reference = ptrvariable->reference;
                                            ptrvariable->counter++;
                                        }
                                    } else {
                                        addTolog("ERROR: La variable no existe en el Struct");
                                    }
                                }else { addTolog("ERROR: Tipo de variable accedida no es compatible");}
                            } else { addTolog("ERROR: Operacion no valida, la referencia no coincide con un Struct"); }
                        } else {
                            addTolog("ERROR: Variable no existe");
                        }

                    }
                }
            }else{
                addTolog("ERROR: Variable no existe");
            }
        }

        else if (key == "cancel") { //Resetear el Pool de memoria
            isupdate = false;
            s_existing.clear();
            globalList.clear();
            ptr_mpool->FreeAllAllocatedMemory();
            in_scope = false;
            in_struct = false;
            active_struct = nullptr;
            deap = 0;
            addTolog("INFO: Reseteo del MemoryPool");
        }

        else if (key == "scope") { // Referente a Scope
            string name = jmessageR.value("name", "oops");
            if (name == "scope_o") { //Se abre Scope
                deap++;
                in_scope = true;
                addTolog("INFO: Scope abierto");
            } else if (name == "scope_c" && in_scope) { //Se cierra Scope
                ptr_mpool->Freeinscope(deap);
                deap--;
                if (deap == 0) {
                    in_scope = false;
                }
                addTolog("INFO: Scope cerrado");
            }
        }
        else if(key == "struct"){ //Referente a Struct
            string value = jmessageR.value("value", "oops");
            string name = jmessageR.value("name", "oops");
            name.erase(name.end()-1);
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (value == "};"){ //Se cierra un Struct
                in_struct = false;
                active_struct = nullptr;
                addTolog("INFO: Struct creado");
            }
            else if (can) { //Si el nombre no esta reservado
                string operation = jmessageR.value("operation", "oops");
                if (operation == "false") { //Declaracion vacia

                    if ( ptr_mpool->GetMemory(1) == NULL){ //Si el Pool no tiene espacio
                        addTolog("FATAL: mServer sin memoria");
                    }
                    else {
                        in_struct = true;
                        void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                        vector<Variable_struct> mstruct;
                        ptrChunk->name = name;
                        ptrChunk->type = "struct";
                        ptrChunk->isReference = false;
                        ptrChunk->mstruct = mstruct;
                        ptrChunk->counter = 1;
                        ptrChunk->isscope = in_scope;
                        ptrChunk->deap = deap;
                        active_struct = ptrChunk;
                        s_existing.push_back(name);
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);
                        addTolog("INFO: Struct inicializado");
                    }

                }
            } else {
                addTolog("ERROR: No se pueden definir dos variables con el mismo nombre");
            }
        }

        for (int i = 0; i < globalList.size(); i++) { //Eliminar variables de la Global List
            if (globalList.at(i).ptr->counter == 0) {
                for (int k = 0; k < s_existing.size(); k++) {
                    if (s_existing.at(k) ==  globalList.at(i).ptr->type){
                        s_existing.erase(std::remove(s_existing.begin(), s_existing.end(), globalList.at(i).ptr->type), s_existing.end());
                        destroyed += globalList.at(i).ptr->type + "$";
                        break;
                    }
                }

                for (int j = i; j < globalList.size() - 1; j++) {
                    globalList[j] = globalList[j + 1];
                }
                i--;
                globalList.erase(globalList.end());
            }
        }
        addTolog("INFO: Ejecutando el Garbage Colletor...");
        ptr_mpool->GarbageCollector();
        if (isupdate) {LtoS();}
        json mymessage = parseJson();
        string message = mymessage.dump();
        send(clientSocket, message.c_str(), message.size() + 1, 0);
        std_out_ = "";
        destroyed = "";
        log_ = "";
        close(clientSocket);

    } //Llave del while

    return 0;
}


int main(){
    bool flag = true;
    string size;
    while (flag){
        cout<< "Ingrese el size del server en bytes: ";
        cin >> size; //Se lee el nombre
        if (stoi(size) <= 54000 && stoi(size) >= 1){
            flag = false;
        }
        else{
            cout<< "Exceso de solicitud de memoria \n";
        }
    }
    flag = true;
    string port;//En esta variable estará almacenado el nombre ingresado.
    while (flag){
        cout<< "Ingrese el puerto de escucha del server: ";
        cin >> port; //Lee el puerto
        if (stoi(port) <= 56000 && stoi(port) >= 54000){
            flag = false;
        }
        else{
            cout<< "Puerto no disponible\n";
        }
    }


    //MALLOC DEL POOL PARA ASIGNACION DE MEMORIA
    MemPool::CMemoryPool *g_ptrMemPool = new MemPool::CMemoryPool(stoi(size), 1, 1, true);
    //Inicializacion del mServer
    startServer(stoi(port), g_ptrMemPool);
    return 0;
}


