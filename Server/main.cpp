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

using json = nlohmann::json;
using namespace std;
bool in_scope = false;

vector<Variable> globalList;
string ram_;
string std_out_;
string log_;
json parseJson (){
    json mymessage =
            {
                    {"std_out_", std_out_},
                    {"ram_", ram_},
                    {"log_", log_},

            };
    return mymessage;

}

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
        if (variable.ptr->type != "char" && variable.ptr->type  != "struct" && !variable.ptr->isReference){
            value = to_string(*variable.ptr->Data);
            ref = to_string(variable.ptr->counter);
        }
        else if (variable.ptr->type == "char" && !variable.ptr->isReference){
            value = string(1, *variable.ptr->Data);
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
    while (true)
    {
    // Wait for a connection

    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

    char host[NI_MAXHOST];      // Client's remote name
    char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

    memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXSERV);

    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        cout << host << " Conected " << endl;
    }
    else
    {
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
    if (bytesReceived == -1)
    {
        cerr << "Error in recv(). Quitting" << endl;
        break;
    }

    if (bytesReceived == 0)
    {
        cout << "Client disconnected " << endl;
        break;
    }
    string messageR = string(buf, 0, bytesReceived);
    json jmessageR = json::parse(messageR);
    cout<< jmessageR<<endl;
    string key = jmessageR.value("key", "oops");
    if (key == "define"){
        string name = jmessageR.value("name", "oops");
        bool can = ptr_mpool->FindChunkHoldingSameName(name);
        if (can){
            string type = jmessageR.value("type", "oops");
            if (type == "int"){
                string operation = jmessageR.value("operation", "oops");
                if (operation == "true"){
                    int *ptrvar = (int *) ptr_mpool->GetMemory(sizeof(int));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    double newvalue = split_getline(value, ptr_mpool);
                    int newvaluei = newvalue;
                    *ptrvar = newvaluei;
                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);

                }
                else if (operation == "reference"){
                    string value = jmessageR.value("value", "oops");
                    value.erase(value.end()-11, value.end());
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                        if(ptrRef->type == type && ptrRef->isReference){
                            int *ptrvar = (int *) ptr_mpool->GetMemory(sizeof(int));   //CREACION DE VARIABLE CON EL POOL CREADO
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter= 1;
                            ptrChunk->isscope = in_scope;
                            cout<< "Reference value: "<< *(int *)ptrRef->reference->Data<<"\n";
                            *ptrvar = *(int *)ptrRef->reference->Data;
                            Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                            globalList.push_back(*variable);
                            LtoS();
                            json mymessage = parseJson();
                            string message = mymessage.dump();
                            send(clientSocket, message.c_str(), message.size() + 1, 0);
                        }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}
                    } else{
                        cout<< "Variable no existe\n";
                    }

                }
                else{
                    int *ptrvar = (int *) ptr_mpool->GetMemory(sizeof(int));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    if(value != "") {
                        *ptrvar = stoi(value);
                    }else{ *ptrvar = NULL;}
                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);


                }
            }else if(type == "long"){
                string operation = jmessageR.value("operation", "oops");
                if (operation == "true"){
                    long *ptrvar = (long *) ptr_mpool->GetMemory(sizeof(long));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    double newvalue = split_getline(value, ptr_mpool);
                    long newvaluei = newvalue;
                    *ptrvar = newvaluei;
                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);

                }
                else if (operation == "reference"){
                    string value = jmessageR.value("value", "oops");
                    value.erase(value.end()-11, value.end());
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                        if(ptrRef->type == type && ptrRef->isReference){
                            long *ptrvar = (long *) ptr_mpool->GetMemory(sizeof(long));   //CREACION DE VARIABLE CON EL POOL CREADO
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter= 1;
                            ptrChunk->isscope = in_scope;
                            cout<< "Reference value: "<< *(long *)ptrRef->reference->Data<<"\n";
                            *ptrvar = *(long *)ptrRef->reference->Data;
                            Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                            globalList.push_back(*variable);
                            LtoS();
                            json mymessage = parseJson();
                            string message = mymessage.dump();
                            send(clientSocket, message.c_str(), message.size() + 1, 0);
                        }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}
                    } else{
                        cout<< "Variable no existe\n";
                    }

                }//CIERRE IF OPERATION
                else{
                    long *ptrvar = (long *) ptr_mpool->GetMemory(sizeof(long));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    if(value != "") {
                        *ptrvar = stol(value);
                    }else{ *ptrvar = NULL;}

                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);
                }

            }else if(type == "float"){
                string operation = jmessageR.value("operation", "oops");
                if (operation == "true"){
                    float *ptrvar = (float *) ptr_mpool->GetMemory(sizeof(float));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    double newvalue = split_getline(value, ptr_mpool);
                    float newvaluei = newvalue;
                    *ptrvar = newvaluei;
                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);

                }
                else if (operation == "reference"){
                    string value = jmessageR.value("value", "oops");
                    value.erase(value.end()-11, value.end());
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                        if(ptrRef->type == type && ptrRef->isReference){
                            float *ptrvar = (float *) ptr_mpool->GetMemory(sizeof(float));   //CREACION DE VARIABLE CON EL POOL CREADO
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter= 1;
                            ptrChunk->isscope = in_scope;
                            cout<< "Reference value: "<< *(float *)ptrRef->reference->Data<<"\n";
                            *ptrvar = *(float *)ptrRef->reference->Data;
                            Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                            globalList.push_back(*variable);
                            LtoS();
                            json mymessage = parseJson();
                            string message = mymessage.dump();
                            send(clientSocket, message.c_str(), message.size() + 1, 0);
                        }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}
                    } else{
                        cout<< "Variable no existe\n";
                    }

                }//CIERRE IF OPERATION
                else{
                    float *ptrvar = (float *) ptr_mpool->GetMemory(sizeof(float));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    if(value != "") {
                        *ptrvar = stof(value);
                    }else{ *ptrvar = NULL;}

                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);
                }

            }else if(type == "double"){
                string operation = jmessageR.value("operation", "oops");
                if (operation == "true"){
                    double *ptrvar = (double *) ptr_mpool->GetMemory(sizeof(double));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    double newvalue = split_getline(value, ptr_mpool);
                    double newvaluei = newvalue;
                    *ptrvar = newvaluei;
                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);

                }
                else if (operation == "reference"){
                    string value = jmessageR.value("value", "oops");
                    value.erase(value.end()-11, value.end());
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                        if(ptrRef->type == type && ptrRef->isReference){
                            double *ptrvar = (double *) ptr_mpool->GetMemory(sizeof(double));   //CREACION DE VARIABLE CON EL POOL CREADO
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter= 1;
                            ptrChunk->isscope = in_scope;
                            cout<< "Reference value: "<< *(double *)ptrRef->reference->Data<<"\n";
                            *ptrvar = *(double *)ptrRef->reference->Data;
                            Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                            globalList.push_back(*variable);
                            LtoS();
                            json mymessage = parseJson();
                            string message = mymessage.dump();
                            send(clientSocket, message.c_str(), message.size() + 1, 0);
                        }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}
                    } else{
                        cout<< "Variable no existe\n";
                    }

                }//CIERRE IF OPERATION
                else{
                    double *ptrvar = (double *) ptr_mpool->GetMemory(sizeof(double));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    ptrChunk->isscope = in_scope;
                    if(value != "") {
                        *ptrvar = stod(value);
                    }else{ *ptrvar = NULL;}

                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);
                }

            }else if(type == "char"){
                string operation = jmessageR.value("operation", "oops");
                if (operation == "true"){
                    string value = jmessageR.value("value", "oops");
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                        cout << ptrRef->type << endl;
                        if (ptrRef->type == type && !ptrRef->isReference) {
                            char *ptrvar = (char *) ptr_mpool->GetMemory(sizeof(char));  //CREACION DE VARIABLE CON EL POOL CREADO
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter = 1;
                            *ptrvar = *ptrRef->Data;
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            LtoS();
                            json mymessage = parseJson();
                            string message = mymessage.dump();
                            send(clientSocket, message.c_str(), message.size() + 1, 0);
                        }
                    }
                }
                else if (operation == "reference"){
                    string value = jmessageR.value("value", "oops");
                    value.erase(value.end()-11, value.end());
                    bool can = ptr_mpool->FindChunkHoldingSameName(value);
                    if (!can) {
                        string type = jmessageR.value("type", "oops");
                        MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
                        cout<<ptrRef->type<<endl;
                        if(ptrRef->type == type && ptrRef->isReference){
                            char *ptrvar = (char *) ptr_mpool->GetMemory(sizeof(char));   //CREACION DE VARIABLE CON EL POOL CREADO
                            MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                            ptrChunk->name = name;
                            ptrChunk->type = type;
                            ptrChunk->counter= 1;
                            cout<< "Reference value: "<< *(char *)ptrRef->reference->Data<<"\n";
                            *ptrvar = *(char *)ptrRef->reference->Data;
                            Variable *variable = new Variable(name, ptrChunk);
                            globalList.push_back(*variable);
                            LtoS();
                            json mymessage = parseJson();
                            string message = mymessage.dump();
                            send(clientSocket, message.c_str(), message.size() + 1, 0);
                        }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}
                    } else{
                        cout<< "Variable no existe\n";
                    }

                }//CIERRE IF OPERATION
                else{
                    char *ptrvar = (char *) ptr_mpool->GetMemory(sizeof(char));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    char value1 = value[1];
                    if(value != "") {
                        *ptrvar = value1;
                    }else{ *ptrvar = NULL;}

                    Variable *variable = new Variable(name, ptrChunk);
                    globalList.push_back(*variable);
                    LtoS();
                    json mymessage = parseJson();
                    string message = mymessage.dump();
                    send(clientSocket, message.c_str(), message.size() + 1, 0);
                }

            }//CIERRE DE IF TYPE
        }
        else{
            cout<<"la madre \n";
        }

    }else if(key == "defineR"){
        string name = jmessageR.value("name", "oops");
        bool can = ptr_mpool->FindChunkHoldingSameName(name);
        if (can) {
            string operation = jmessageR.value("operation", "oops");
            if(operation == "true"){
                string value = jmessageR.value("value", "oops");
                value.erase(value.end()-10, value.end());
                bool can = ptr_mpool->FindChunkHoldingSameName(value);
                if (!can) {
                    string type = jmessageR.value("type", "oops");
                    MemPool::SMemoryChunk *ptrOrig = ptr_mpool->FindChunkHoldingNameTo(value);
                    if(ptrOrig->type == type && !ptrOrig->isReference){
                        void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                        MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                        ptrChunk->name = name;
                        ptrChunk->type = type;
                        ptrChunk->isReference = true;
                        ptrChunk->reference = ptrOrig;
                        ptrChunk->counter= -1;
                        ptrChunk->isscope = in_scope;
                        ptrOrig->counter++;
                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                        globalList.push_back(*variable);
                        LtoS();
                        json mymessage = parseJson();
                        string message = mymessage.dump();
                        send(clientSocket, message.c_str(), message.size() + 1, 0);
                    }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}
                } else{
                    cout<< "Variable no existe\n";
                }

            }else{
                string value = jmessageR.value("value", "oops");
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
                        ptrChunk->counter= -1;
                        ptrChunk->isscope = in_scope;
                        ptrOrig->counter++;
                        Variable *variable = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                        globalList.push_back(*variable);
                        LtoS();
                        json mymessage = parseJson();
                        string message = mymessage.dump();
                        send(clientSocket, message.c_str(), message.size() + 1, 0);
                    }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}}
                else{
                    cout<< "Variable no existe\n";
                }
            }

        }
    }
    else if(key == "print"){
        string value = jmessageR.value("value", "oops");
        bool can = ptr_mpool->FindChunkHoldingSameName(value);
        if (!can) {
            MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(value);
            string type = ptrRef->type;
            if (type != "char" && type != "struct" && !ptrRef->isReference){
                //int valueprint = *((int *) ptrRef->Data);
                std_out_ = to_string(*ptrRef->Data) + "\n";
                json mymessage = parseJson();
                string message = mymessage.dump();
                send(clientSocket, message.c_str(), message.size() + 1, 0);
            }
            else if (type == "char" && !ptrRef->isReference){
                //long valueprint = *((long *) ptrRef->Data);
                std_out_ = string(1, *ptrRef->Data) + "\n";
                json mymessage = parseJson();
                string message = mymessage.dump();
                send(clientSocket, message.c_str(), message.size() + 1, 0);
            }
        }
        else{
            cout<< "Variable no existe\n";
        }

    }
    else if(key == "cancel"){
        globalList.clear();
        ptr_mpool->FreeAllAllocatedMemory();
        json mymessage = parseJson();
        string message = mymessage.dump();
        send(clientSocket, message.c_str(), message.size() + 1, 0);
    }
    else if (key== "scope"){
        string name = jmessageR.value("name", "oops");
        if (name == "scope_o" && !in_scope){
            in_scope = true;
            cout<< "izi\n";
        }
        if (name == "scope_c" && in_scope){
            ptr_mpool->Freeinscope();
            in_scope = false;

        }
    }
    for (int i= 0; i< globalList.size(); i++){
        if (globalList.at(i).ptr->counter == 0){
            for (int j= i; j< globalList.size()-1; j++){
                globalList [j] = globalList [j+1];
            }
            i--;
            globalList.erase(globalList.end());
        }
    }
    ptr_mpool->GarbageCollector();

    // Echo message back to client
    //send(clientSocket, buf, bytesReceived + 1, 0);

    close(clientSocket);
    }

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


