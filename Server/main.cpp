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


using json = nlohmann::json;
using namespace std;
bool in_scope = false;
bool in_struct = false;
int deap= 0;
vector<string> s_existing;
vector<Variable> globalList;
MemPool::SMemoryChunk * active_struct;
string ram_;
string std_out_;
string log_;
string destroyed;

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

void LtoS(){
    cout<< "LtoS" << endl;
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
        //otros casos

        ram_ +=" "+address+" / "+name+" / "+value+" / "+ref+" \n";
        cout<< ram_<< endl;
    }
}

int startServer(int port, MemPool::CMemoryPool *ptr_mpool) {
    // Create a socket

    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1)
    {
        cerr << "Can't create a socket! Quitting" << endl;
        return -1;
    }

    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);

    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
    bind(listening, (sockaddr*)&hint, sizeof(hint));
    // Tell Winsock the socket is for listening
    listen(listening, SOMAXCONN);
    while (true) {
        // Wait for a connection

        sockaddr_in client;
        socklen_t clientSize = sizeof(client);

        int clientSocket = accept(listening, (sockaddr *) &client, &clientSize);

        char host[NI_MAXHOST];      // Client's remote name
        char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

        memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

        if (getnameinfo((sockaddr *) &client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            cout << host << " Conected " << endl;
        } else {
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            cout << host << " connected on port " << ntohs(client.sin_port) << endl;
        }

        // Close listening socket
        //close(listening);

        // While loop: accept and echo message back to client
        char buf[4096];


        memset(buf, 0, 4096);

        // Wait for client to send data
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == -1) {
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }

        if (bytesReceived == 0) {
            cout << "Client disconnected " << endl;
            break;
        }
        string messageR = string(buf, 0, bytesReceived);
        json jmessageR = json::parse(messageR);
        cout << jmessageR << endl;
        string key = jmessageR.value("key", "oops");
        if (key == "define") {
            string name = jmessageR.value("name", "oops");
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (can) {
                string type = jmessageR.value("type", "oops");
                if (type == "int") {
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") {
                        int *ptrvar = (int *) ptr_mpool->GetMemory(
                                sizeof(int));  //CREACION DE VARIABLE CON EL POOL CREADO
                        string value = jmessageR.value("value", "oops");
                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                        ptrChunk->name = name;
                        ptrChunk->type = type;
                        ptrChunk->counter = 1;
                        ptrChunk->isscope = in_scope;
                        ptrChunk->deap = deap;
                        double newvalue = split_getline(value, ptr_mpool);
                        int newvaluei = newvalue;
                        *ptrvar = newvaluei;
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);


                    } else if (operation == "reference") {
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) {
                                int *ptrvar = (int *) ptr_mpool->GetMemory(
                                        sizeof(int));   //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                cout << "Reference value: " << *(int *) ptrRef->reference->Data << "\n";
                                *ptrvar = *(int *) ptrRef->reference->Data;
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                globalList.push_back(*variable);

                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }

                    } else if(operation == "struct"){
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){
                                        int *ptrvar = (int *) ptr_mpool->GetMemory(
                                                sizeof(int));   //CREACION DE VARIABLE CON EL POOL CREADO
                                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                        ptrChunk->name = name;
                                        ptrChunk->type = type;
                                        ptrChunk->counter = 1;
                                        ptrChunk->isscope = in_scope;
                                        ptrChunk->deap = deap;
                                        *ptrvar = *(int *) ptrvariable->Data;
                                        if (in_struct){
                                            ptrChunk->isstruct = true;
                                            Variable_struct *variableStruct = new Variable_struct(name);
                                            active_struct->mstruct.push_back(*variableStruct);
                                        }
                                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                        globalList.push_back(*variable);
                                    }
                                }
                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }
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
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        cout << "out struct\n";
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);


                    }
                }
                else if (type == "long") {
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") {
                        long *ptrvar = (long *) ptr_mpool->GetMemory(
                                sizeof(long));  //CREACION DE VARIABLE CON EL POOL CREADO
                        string value = jmessageR.value("value", "oops");
                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                        ptrChunk->name = name;
                        ptrChunk->type = type;
                        ptrChunk->counter = 1;
                        ptrChunk->isscope = in_scope;
                        ptrChunk->deap = deap;
                        double newvalue = split_getline(value, ptr_mpool);
                        long newvaluei = newvalue;
                        *ptrvar = newvaluei;
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);


                    } else if (operation == "reference") {
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) {
                                long *ptrvar = (long *) ptr_mpool->GetMemory(
                                        sizeof(long));   //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                cout << "Reference value: " << *(long *) ptrRef->reference->Data << "\n";
                                *ptrvar = *(long *) ptrRef->reference->Data;
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                globalList.push_back(*variable);

                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }

                    }else if(operation == "struct"){
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){
                                        long *ptrvar = (long *) ptr_mpool->GetMemory(
                                                sizeof(long));   //CREACION DE VARIABLE CON EL POOL CREADO
                                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                        ptrChunk->name = name;
                                        ptrChunk->type = type;
                                        ptrChunk->counter = 1;
                                        ptrChunk->isscope = in_scope;
                                        ptrChunk->deap = deap;
                                        *ptrvar = *(long *) ptrvariable->Data;
                                        if (in_struct){
                                            ptrChunk->isstruct = true;
                                            Variable_struct *variableStruct = new Variable_struct(name);
                                            active_struct->mstruct.push_back(*variableStruct);
                                        }
                                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                        globalList.push_back(*variable);
                                    }
                                }
                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }
                    }
                    //CIERRE IF OPERATION
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
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);

                    }

                }
                else if (type == "float") {
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") {
                        float *ptrvar = (float *) ptr_mpool->GetMemory(
                                sizeof(float));  //CREACION DE VARIABLE CON EL POOL CREADO
                        string value = jmessageR.value("value", "oops");
                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                        ptrChunk->name = name;
                        ptrChunk->type = type;
                        ptrChunk->counter = 1;
                        ptrChunk->isscope = in_scope;
                        ptrChunk->deap = deap;
                        double newvalue = split_getline(value, ptr_mpool);
                        float newvaluei = newvalue;
                        *ptrvar = newvaluei;
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);


                    } else if (operation == "reference") {
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) {
                                float *ptrvar = (float *) ptr_mpool->GetMemory(
                                        sizeof(float));   //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                cout << "Reference value: " << *(float *) ptrRef->reference->Data << "\n";
                                *ptrvar = *(float *) ptrRef->reference->Data;
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                globalList.push_back(*variable);

                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }

                    }else if(operation == "struct"){
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){
                                        float *ptrvar = (float *) ptr_mpool->GetMemory(
                                                sizeof(float));   //CREACION DE VARIABLE CON EL POOL CREADO
                                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                        ptrChunk->name = name;
                                        ptrChunk->type = type;
                                        ptrChunk->counter = 1;
                                        ptrChunk->isscope = in_scope;
                                        ptrChunk->deap = deap;
                                        *ptrvar = *(float *) ptrvariable->Data;
                                        if (in_struct){
                                            ptrChunk->isstruct = true;
                                            Variable_struct *variableStruct = new Variable_struct(name);
                                            active_struct->mstruct.push_back(*variableStruct);
                                        }
                                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                        globalList.push_back(*variable);
                                    }
                                }
                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }
                    }//CIERRE IF OPERATION
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
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);

                    }

                }
                else if (type == "double") {
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") {
                        double *ptrvar = (double *) ptr_mpool->GetMemory(
                                sizeof(double));  //CREACION DE VARIABLE CON EL POOL CREADO
                        string value = jmessageR.value("value", "oops");
                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                        ptrChunk->name = name;
                        ptrChunk->type = type;
                        ptrChunk->counter = 1;
                        ptrChunk->isscope = in_scope;
                        ptrChunk->deap = deap;
                        double newvalue = split_getline(value, ptr_mpool);
                        double newvaluei = newvalue;
                        *ptrvar = newvaluei;
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);


                    } else if (operation == "reference") {
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) {
                                double *ptrvar = (double *) ptr_mpool->GetMemory(
                                        sizeof(double));   //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                cout << "Reference value: " << *(double *) ptrRef->reference->Data << "\n";
                                *ptrvar = *(double *) ptrRef->reference->Data;
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                globalList.push_back(*variable);

                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }

                    }else if(operation == "struct"){
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){
                                        double *ptrvar = (double *) ptr_mpool->GetMemory(
                                                sizeof(double));   //CREACION DE VARIABLE CON EL POOL CREADO
                                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                        ptrChunk->name = name;
                                        ptrChunk->type = type;
                                        ptrChunk->counter = 1;
                                        ptrChunk->isscope = in_scope;
                                        ptrChunk->deap = deap;
                                        *ptrvar = *(double *) ptrvariable->Data;
                                        if (in_struct){
                                            ptrChunk->isstruct = true;
                                            Variable_struct *variableStruct = new Variable_struct(name);
                                            active_struct->mstruct.push_back(*variableStruct);
                                        }
                                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                        globalList.push_back(*variable);
                                    }
                                }
                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }
                    }//CIERRE IF OPERATION
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
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);

                    }

                }
                else if (type == "char") {
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "true") {
                        string value = jmessageR.value("value", "oops");
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            cout << ptrRef->type << endl;
                            if (ptrRef->type == type && !ptrRef->isReference) {
                                char *ptrvar = (char *) ptr_mpool->GetMemory(
                                        sizeof(char));  //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                *ptrvar = *ptrRef->Data;
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);

                            }
                        }
                    } else if (operation == "reference") {
                        string value = jmessageR.value("value", "oops");
                        value.erase(value.end() - 11, value.end());
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            cout << ptrRef->type << endl;
                            if (ptrRef->type == type && ptrRef->isReference && !ptrRef->isstruct) {
                                char *ptrvar = (char *) ptr_mpool->GetMemory(
                                        sizeof(char));   //CREACION DE VARIABLE CON EL POOL CREADO
                                MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                ptrChunk->name = name;
                                ptrChunk->type = type;
                                ptrChunk->counter = 1;
                                ptrChunk->isscope = in_scope;
                                ptrChunk->deap = deap;
                                *ptrvar = *(char *) ptrRef->reference->Data;
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);

                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }

                    }else if(operation == "struct"){
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){
                                        char *ptrvar = (char *) ptr_mpool->GetMemory(
                                                sizeof(char));   //CREACION DE VARIABLE CON EL POOL CREADO
                                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                        ptrChunk->name = name;
                                        ptrChunk->type = type;
                                        ptrChunk->counter = 1;
                                        ptrChunk->isscope = in_scope;
                                        ptrChunk->deap = deap;
                                        *ptrvar = *(char *) ptrvariable->Data;
                                        if (in_struct){
                                            ptrChunk->isstruct = true;
                                            Variable_struct *variableStruct = new Variable_struct(name);
                                            active_struct->mstruct.push_back(*variableStruct);
                                        }
                                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                        globalList.push_back(*variable);
                                    }
                                }
                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }
                    }//CIERRE IF OPERATION
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
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);

                    }

                }//CIERRE DE IF TYPE
                else if(validenamestruct(type)){
                    string operation = jmessageR.value("operation", "oops");
                    if (operation == "false"){
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
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);
                        globalList.push_back(*variable);
                    }
                    else if(operation == "true"){
                        string value = jmessageR.value("value", "oops");
                        if(!ptr_mpool->FindChunkHoldingSameName(value)){
                            MemPool::SMemoryChunk *ptrRef =ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == type){
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
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);
                                globalList.push_back(*variable);
                            }
                        }
                    }else if(operation == "struct"){
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type)) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == type){
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){
                                        void *ptrvar = ptr_mpool->GetMemory(1);   //CREACION DE VARIABLE CON EL POOL CREADO
                                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                        ptrChunk->name = name;
                                        ptrChunk->type = type;
                                        ptrChunk->counter = 1;
                                        ptrChunk->isscope = in_scope;
                                        ptrChunk->deap = deap;
                                        ptrChunk->reference = ptrvariable->reference;
                                        ptrChunk->isReference = true;
                                        if (in_struct){
                                            ptrChunk->isstruct = true;
                                            Variable_struct *variableStruct = new Variable_struct(name);
                                            active_struct->mstruct.push_back(*variableStruct);
                                        }
                                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                        globalList.push_back(*variable);
                                    }
                                }
                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }
                    }
                }
            } else {
                cout << "la madre \n";
            }

        }
        else if (key == "defineR") {
            string name = jmessageR.value("name", "oops");
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (can) {
                string operation = jmessageR.value("operation", "oops");
                if (operation == "true") {
                    string value = jmessageR.value("value", "oops");
                    value.erase(value.end() - 10, value.end());
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrOrig = ptr_mpool->FindChunkHoldingNameTo(value);
                        if (ptrOrig->type == type && !ptrOrig->isReference && !ptrOrig->isstruct) {
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
                            if (in_struct){
                                ptrChunk->isstruct = true;
                                Variable_struct *variableStruct = new Variable_struct(name);
                                active_struct->mstruct.push_back(*variableStruct);
                            }
                            Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                            globalList.push_back(*variable);

                        } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                    } else {
                        cout << "Variable no existe\n";
                    }

                }else if(operation == "struct"){
                    string value = jmessageR.value("value", "oops");
                    QString Qvalue = QString::fromStdString(value);
                    QStringList parts = Qvalue.split(".");
                    string struct_temp = parts.at(0).toStdString();
                    string variable_temp = parts.at(1).toStdString();
                    bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                        if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                            MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                            if (ptrvariable->type == type){
                                bool flag =false;
                                for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                    if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                        flag = true;
                                        break;
                                    }
                                }
                                if (flag){
                                    void *ptrvar = ptr_mpool->GetMemory(1);   //CREACION DE VARIABLE CON EL POOL CREADO
                                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                                    ptrChunk->name = name;
                                    ptrChunk->type = type;
                                    ptrChunk->counter = 1;
                                    ptrChunk->isscope = in_scope;
                                    ptrChunk->deap = deap;
                                    ptrChunk->reference = ptrvariable->reference;
                                    ptrChunk->isReference = true;
                                    if (in_struct){
                                        ptrChunk->isstruct = true;
                                        Variable_struct *variableStruct = new Variable_struct(name);
                                        active_struct->mstruct.push_back(*variableStruct);
                                    }
                                    Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                    globalList.push_back(*variable);
                                }
                            }
                        } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                    } else {
                        cout << "Variable no existe\n";
                    }
                }
                else {
                    string value = jmessageR.value("value", "oops");
                    if (value != "") {
                        bool can = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->isReference && ptrRef->type == type) {
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
                                if (in_struct){
                                    ptrChunk->isstruct = true;
                                    Variable_struct *variableStruct = new Variable_struct(name);
                                    active_struct->mstruct.push_back(*variableStruct);
                                }
                                Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                                globalList.push_back(*variable);

                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }
                    } else {
                        string type = jmessageR.value("type", "oops");
                        void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                        ptrChunk->name = name;
                        ptrChunk->type = type;
                        ptrChunk->isReference = true;
                        ptrChunk->counter = -1;
                        ptrChunk->isscope = in_scope;
                        ptrChunk->deap = deap;
                        if (in_struct){
                            ptrChunk->isstruct = true;
                            Variable_struct *variableStruct = new Variable_struct(name);
                            active_struct->mstruct.push_back(*variableStruct);
                        }
                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                        globalList.push_back(*variable);



                    }

                }

            }
        }
        else if (key == "print") {
            string value = jmessageR.value("value", "oops");
            bool can = ptr_mpool->FindChunkHoldingSameName(value);
            if (!can) {
                MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                string type = ptrRef->type;
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
            } else {
                cout << "Variable no existe\n";

            }

        }
        else if (key == "equal") {
            string name = jmessageR.value("name", "oops");
            QString Qname = QString::fromStdString(name);
            QStringList parts_n = Qname.split(".");
            if (parts_n.length() == 2){
                cout << "1" << endl;
                name = parts_n.at(1).toStdString();
            }
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (!can) {
                cout << name << endl;
                MemPool::SMemoryChunk *ptrOrg = ptr_mpool->FindChunkHoldingNameTo(name);
                if (!ptrOrg->isReference) {
                    string type = jmessageR.value("type", "oops");
                    string value = jmessageR.value("value", "oops");
                    if (type == "variable") {
                        try {
                            cout << "3" << endl;
                            if (ptrOrg->type == "char" && string(1, value[0]) == "'" && string(1, value[2]) == "'" &&
                                value.length() == 3) {
                                *ptrOrg->Data = value[1];
                            }
                            else if (ptrOrg->type == "int" && value.length() == to_string(stoi(value)).length()) {
                                *ptrOrg->Data = stoi(value);
                                cout << "4" << endl;
                            } else if (ptrOrg->type == "long" && value.length() == to_string(stol(value)).length()) {
                                *ptrOrg->Data = stol(value);
                            } else if (ptrOrg->type == "float" && QString::fromStdString(value).split(".").length() ==2 &&
                                       QString::fromStdString(value).split(".").at(0).toStdString().length() ==
                                       to_string(stol(QString::fromStdString(value).split(".").at(
                                               0).toStdString())).length()
                                       && QString::fromStdString(value).split(".").at(1).toStdString().length() ==
                                          to_string(stol(QString::fromStdString(value).split(".").at(
                                                  1).toStdString())).length()) {
                                *ptrOrg->Data = stof(value);
                            } else if (ptrOrg->type == "double" && QString::fromStdString(value).split(".").length() ==2 &&
                                       QString::fromStdString(value).split(".").at(0).toStdString().length() ==
                                       to_string(stol(QString::fromStdString(value).split(".").at(
                                               0).toStdString())).length()
                                       && QString::fromStdString(value).split(".").at(1).toStdString().length() ==
                                          to_string(stol(QString::fromStdString(value).split(".").at(
                                                  1).toStdString())).length()) {
                                *ptrOrg->Data = stod(value);

                            }else {
                                stod("a");
                            }

                        } catch (std::invalid_argument) {
                            QString Qvalue = QString::fromStdString(value);
                            QStringList parts_v = Qvalue.split(".");
                            if (parts_v.length() == 2){
                                string struct_temp = parts_v.at(0).toStdString();
                                string variable_temp = parts_v.at(1).toStdString();
                                bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                                if (!can) {
                                    string type = jmessageR.value("type", "oops");
                                    MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                                    if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                        MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                        if (ptrvariable->type == ptrOrg->type){
                                            bool flag =false;
                                            for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                                if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                                    flag = true;
                                                    break;
                                                }
                                            }
                                            if (flag){
                                                if (ptrOrg->type != "struct" && ptrOrg->type != "reference") {
                                                    *ptrOrg->Data = *ptrvariable->Data;
                                                }
                                            }
                                        }
                                    } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                                } else {
                                    cout << "Variable no existe\n";
                                }
                            }
                            else{
                                bool isVariable = ptr_mpool->FindChunkHoldingSameName(value);
                                if (!isVariable) {
                                    MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                                    if (ptrOrg->type == ptrRef->type) {
                                        *ptrOrg->Data = *ptrRef->Data;

                                    } else { cout << "NO FUNKO MAI BRO" << endl; }
                                }
                            }

                        }

                    } else if (type == "reference") {
                        value.erase(value.end() - 11, value.end());
                        bool valide = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!valide) {
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == ptrOrg->type && ptrRef->isReference && !ptrRef->isstruct) {
                                *ptrOrg->Data = *ptrRef->reference->Data;

                            } else {
                                cout << "Ayuda" << endl;
                            }
                        }

                    } else if (type == "operation") {
                        double newvalue = split_getline(value, ptr_mpool);
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
                        }
                    }
                } else if (ptrOrg->isReference && ptrOrg->type !="struct") {
                    string type = jmessageR.value("type", "oops");
                    string value = jmessageR.value("value", "oops");
                    if (type == "variable") {
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        if (parts.length() == 2){
                            cout << "parts\n";
                            string struct_temp = parts.at(0).toStdString();
                            string variable_temp = parts.at(1).toStdString();
                            bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                            if (!can) {
                                string type = jmessageR.value("type", "oops");
                                MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                                if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                    MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                    if (ptrvariable->type == ptrOrg->type){
                                        bool flag =false;
                                        for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                            if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                                flag = true;
                                                break;
                                            }
                                        }
                                        if (flag){
                                            if (ptrOrg->reference != NULL) {
                                                ptrOrg->reference->counter--;
                                            }
                                            ptrvariable->reference->counter++;
                                            ptrOrg->reference = ptrvariable->reference;
                                        }
                                    }
                                } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                            } else {
                                cout << "Variable no existe\n";
                            }
                        }
                        else{
                            bool isVariable = ptr_mpool->FindChunkHoldingSameName(value);
                            if (!isVariable) {
                                MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                                if (ptrOrg->type == ptrRef->type) {
                                    if (ptrOrg->reference != NULL) {
                                        ptrOrg->reference->counter--;
                                    }
                                    ptrRef->reference->counter++;
                                    ptrOrg->reference = ptrRef->reference;
                                }
                            }
                        }

                    } else if (type == "reference") {
                        value.erase(value.end() - 10, value.end());
                        bool valide = ptr_mpool->FindChunkHoldingSameName(value);
                        if (!valide) {
                            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                            if (ptrRef->type == ptrOrg->type && !ptrRef->isReference && !ptrRef->isstruct) {
                                if (ptrOrg->reference != NULL) {
                                    ptrOrg->reference->counter--;
                                }
                                ptrRef->counter++;
                                ptrOrg->reference = ptrRef;

                            } else {
                                cout << "Ayuda2" << endl;
                            }
                        }
                    }
                    else if(type == "operation"){
                        string value = jmessageR.value("value", "oops");
                        QString Qvalue = QString::fromStdString(value);
                        QStringList parts = Qvalue.split(".");
                        string struct_temp = parts.at(0).toStdString();
                        string variable_temp = parts.at(1).toStdString();
                        bool can = ptr_mpool->FindChunkHoldingSameName(struct_temp);
                        if (!can) {
                            string type = jmessageR.value("type", "oops");
                            MemPool::SMemoryChunk *ptrstruct = ptr_mpool->FindChunkHoldingNameTo(struct_temp);
                            if (validenamestruct(ptrstruct->type) && ptrstruct->isReference && !ptrstruct->isstruct) {
                                MemPool::SMemoryChunk *ptrvariable = ptr_mpool->FindChunkHoldingNameTo(variable_temp);
                                if (ptrvariable->type == ptrOrg->type){
                                    bool flag =false;
                                    for (int i=0; i<ptrstruct->reference->mstruct.size(); i++){
                                        if (ptrstruct->reference->mstruct.at(i).name == variable_temp){
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag){
                                        if (ptrOrg->type != "struct" && ptrOrg->type != "reference") {
                                            *ptrOrg->Data = *ptrvariable->Data;
                                            ptrvariable->counter++;
                                        }
                                        else if( ptrOrg->type == "reference"){
                                            ptrOrg->reference = ptrvariable->reference;
                                            ptrvariable->counter++;
                                        }
                                    }
                                }
                            } else { cout << "LAS VARIABLES NO COINCIDEN EN EL TIPO\n"; }
                        } else {
                            cout << "Variable no existe\n";
                        }

                    }
                }
            }
        }
        else if (key == "cancel") {
            s_existing.clear();
            globalList.clear();
            ptr_mpool->FreeAllAllocatedMemory();

        }
        else if (key == "scope") {
            string name = jmessageR.value("name", "oops");
            if (name == "scope_o") {
                deap++;
                in_scope = true;
            } else if (name == "scope_c" && in_scope) {
                ptr_mpool->Freeinscope(deap);
                deap--;
                if (deap == 0) {
                    in_scope = false;
                }
            }
        }
        else if(key == "struct"){
            string value = jmessageR.value("value", "oops");
            string name = jmessageR.value("name", "oops");
            name.erase(name.end()-1);
            bool can = ptr_mpool->FindChunkHoldingSameName(name);
            if (value == "};"){
                in_struct = false;
                active_struct = nullptr;
            }
            else if (can) {
                string operation = jmessageR.value("operation", "oops");
                if (operation == "false") {
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
                    Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                    globalList.push_back(*variable);

                }
            }
        }
        for (int i = 0; i < globalList.size(); i++) {
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
        ptr_mpool->GarbageCollector();
        LtoS();
        json mymessage = parseJson();
        string message = mymessage.dump();
        send(clientSocket, message.c_str(), message.size() + 1, 0);
        std_out_ = "";
        destroyed = "";
        close(clientSocket);
    } //Llave del while

    // Close the socket

    return 0;
}



int main(){
    bool flag = true;
    string size;
    while (flag){
        cout<< "Ingrese el size del server en bits: ";
        cin >> size; //Se lee el nombre
        if (stoi(size) <= 54000){
            flag = false;
        }
        else{
            cout<< "Exceso de solicitud de memoria \n";
        }
    }

    cout<< "Ingrese el puerto de escucha del server: ";
    string port;//En esta variable estará almacenado el nombre ingresado.
    cin >> port; //Se lee el nombre
    //MALLOC DEL POOL PARA ASIGNACION DE MEMORIA
    MemPool::CMemoryPool *g_ptrMemPool = new MemPool::CMemoryPool(stoi(size), 1, 1, true);
    startServer(stoi(port), g_ptrMemPool);
    return 0;
}


