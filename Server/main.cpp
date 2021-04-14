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

list<Variable*> globalList;

using json = nlohmann::json;
using namespace std;



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
        cout << host << " connected on port " << service << endl;
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
    cout << messageR << endl;
    json jmessageR = json::parse(messageR);
    string key = jmessageR.value("key", "oops");
    if (key == "define"){
        string type = jmessageR.value("key", "oops");
        string name = jmessageR.value("name", "oops");
        if (type == "int"){
            //list<Variable*>::iterator pos;
            Variable *pos;
            bool can = true;
            for(int i =0; i >= globalList.size(); i++){
                pos = *globalList.begin();
                if (pos->name == name){
                    cout <<"Error en el nombre de la variable, ya existe";
                    can = false;
                    break;
                }
                pos++;
            }
            if (can) {
                int *ptrint = (int *) ptr_mpool->GetMemory(sizeof(int));  //CREACION DE VARIABLE CON EL POOL CREADO
                string value = jmessageR.value("value", "oops");
                *ptrint = stoi(value);
                string type = jmessageR.value("type", "oops");

                Variable *myvar = new Variable(type, name, ptrint, 1);
                globalList.push_back(myvar);
                cout << *globalList.begin() << "\n";

                send(clientSocket, buf, bytesReceived + 1, 0);
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
    std::thread ser (startServer, stoi(port), g_ptrMemPool);
    ser.detach();
    return 0;
}


