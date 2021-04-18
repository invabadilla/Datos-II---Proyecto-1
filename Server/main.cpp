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

using json = nlohmann::json;
using namespace std;

list<Variable> globalList;

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
                    double newvalue = split_getline(value, ptr_mpool);
                    cout << "Double: " << newvalue << endl;
                    int newvaluei = newvalue;
                    cout << "int: " << newvaluei << endl;
                    *ptrvar = newvaluei;
                    cout << "Desreferencia: " << *ptrChunk->Data << endl;
                    Variable *a = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                    globalList.push_back(*a);
                }
                else{
                    int *ptrvar = (int *) ptr_mpool->GetMemory(sizeof(int));  //CREACION DE VARIABLE CON EL POOL CREADO
                    string value = jmessageR.value("value", "oops");
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->counter = 1;
                    *ptrvar = stoi(value);
                    cout << "Desreferencia: " << *ptrChunk->Data << endl;
                    Variable *a = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                    globalList.push_back(*a);


                }
                send(clientSocket, buf, bytesReceived + 1, 0);
            }
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
                string type = jmessageR.value("type", "oops");
                MemPool::SMemoryChunk *ptrOrig = ptr_mpool->FindChunkHoldingNameTo(name);

                if(ptrOrig->type == type){
                    void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->isReference = true;
                    ptrChunk->reference = ptrOrig;
                    ptrOrig->counter++;
                    Variable *a = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                    globalList.push_back(*a);
                }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}

            }else{
                string type = jmessageR.value("type", "oops");
                MemPool::SMemoryChunk *ptrRef = ptr_mpool->FindChunkHoldingNameTo(name);
                if (ptrRef->isReference && ptrRef->type == type) {
                    MemPool::SMemoryChunk *ptrOrig = ptrRef->reference;

                    void *ptrvar = ptr_mpool->GetMemory(1);  //CREACION DE VARIABLE CON EL POOL CREADO
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingPointerTo(ptrvar);
                    ptrChunk->name = name;
                    ptrChunk->type = type;
                    ptrChunk->isReference = true;
                    ptrChunk->reference = ptrOrig;
                    ptrOrig->counter++;
                    Variable *a = new Variable(name, ptrChunk);        //NO ESTOY SEGURA JEJEPS
                    globalList.push_back(*a);
                }else{ cout<< "LAS VARIABLES NO COINCIDEN EN EL TIPO\n";}
            }

        }
    }
    // Echo message back to client
    //send(clientSocket, buf, bytesReceived + 1, 0);

    close(clientSocket);
    }

    // Close the socket

    return 0;
}



int main(){


    cout<< "Ingrese el size del server en bits: ";
    string size;//En esta variable estará almacenado el nombre ingresado.
    cin >> size; //Se lee el nombre
    cout<< "Ingrese el puerto de escucha del server: ";
    string port;//En esta variable estará almacenado el nombre ingresado.
    cin >> port; //Se lee el nombre
    //MALLOC DEL POOL PARA ASIGNACION DE MEMORIA
    MemPool::CMemoryPool *g_ptrMemPool = new MemPool::CMemoryPool(stoi(size), 1, 1, true);
    startServer(stoi(port), g_ptrMemPool);
    return 0;
}


