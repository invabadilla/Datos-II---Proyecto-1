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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <iostream>
#include <fstream>

using namespace std;

int startServer(int port) {
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

    cout << string(buf, 0, bytesReceived) << endl;

    // Echo message back to client
    send(clientSocket, buf, bytesReceived + 1, 0);

    close(clientSocket);
    }

    // Close the socket

    return 0;
}

/**int main()
{
    std::thread ser (startServer, 54000);
    ser.join();
    return 0;
}**/

MemPool::CMemoryPool *g_ptrMemPool = NULL  ; //!< Global MemoryPool (Testing purpose)
unsigned int TestCount             = 50000 ; //!< Nr of (de-)allocations (Testing purpose)
unsigned int ArraySize             = 1000  ; //!< Size of the "Testing"-Array

/*! \class MyTestClass_OPOverload
 *  \brief Test Class (Operator new/delete overloaded)
 *
 * The only purpose of this class is the testing of the Memory-Pool.
 */
class MyTestClass_OPOverload
{
public :
    MyTestClass_OPOverload()
    {
        m_cMyArray[0] = 'H' ;
        m_cMyArray[1] = 'e' ;
        m_cMyArray[2] = 'l' ;
        m_cMyArray[3] = 'l' ;
        m_cMyArray[4] = 'o' ;
        m_cMyArray[5] = NULL ;
        m_strMyString = "This is a small Test-String" ;
        m_iMyInt = 12345 ;

        m_fFloatValue = 23456.7890f ;
        m_fDoubleValue = 6789.012345 ;

        Next = this ;
    }

    virtual ~MyTestClass_OPOverload() {} ;

    void *operator new(std::size_t ObjectSize)
    {
        return g_ptrMemPool->GetMemory(ObjectSize) ;
    }

    void operator delete(void *ptrObject, std::size_t ObjectSize)
    {
        g_ptrMemPool->FreeMemory(ptrObject, ObjectSize) ;
    }
private :
    // Test-Data
    char m_cMyArray[25] ;
    unsigned char m_BigArray[10000] ;
    std::string m_strMyString ;
    int m_iMyInt ;
    MyTestClass_OPOverload *Next ;
    float m_fFloatValue ;
    double m_fDoubleValue ;
} ;

/*! \class MyTestClass
 *  \brief Test Class ("Original" new/delete operator)
 *
 * The only purpose of this class is the testing of the Memory-Pool.
 */
class MyTestClass
{
public :
    MyTestClass()
    {
        m_cMyArray[0] = 'H' ;
        m_cMyArray[1] = 'e' ;
        m_cMyArray[2] = 'l' ;
        m_cMyArray[3] = 'l' ;
        m_cMyArray[4] = 'o' ;
        m_cMyArray[5] = NULL ;
        m_strMyString = "This is a small Test-String" ;
        m_iMyInt = 12345 ;

        m_fFloatValue = 23456.7890f ;
        m_fDoubleValue = 6789.012345 ;

        Next = this ;
    }

    virtual ~MyTestClass() {} ;
private :
    // Test-Data
    char m_cMyArray[25] ;
    unsigned char m_BigArray[10000] ;
    std::string m_strMyString ;
    int m_iMyInt ;
    MyTestClass *Next ;
    float m_fFloatValue ;
    double m_fDoubleValue ;
} ;

/******************
CreateGlobalMemPool
******************/
void CreateGlobalMemPool()
{
    std::cerr << "Creating MemoryPool...." ;
    g_ptrMemPool = new MemPool::CMemoryPool(100000, 10000, 2, true) ;
    std::cerr << "OK" << std::endl ;
}

/******************
DestroyGlobalMemPool
******************/
void DestroyGlobalMemPool()
{
    std::cerr << "Deleting MemPool...." ;
    if(g_ptrMemPool) delete g_ptrMemPool ;
    std::cerr << "OK" << std::endl ;
}

/******************
TestAllocationSpeedClassMemPool
******************/
void TestAllocationSpeedClassMemPool()
{
    std::cerr << "Allocating Memory (Object Size : " << sizeof(MyTestClass_OPOverload) << ")..." ;


    for(unsigned int j = 0; j < TestCount; j++)
    {
        MyTestClass_OPOverload *ptrTestClass = new MyTestClass_OPOverload ;
        delete ptrTestClass ;
    }

    std::cerr << "OK" << std::endl ;

}

/******************
TestAllocationSpeedClassHeap
******************/
void TestAllocationSpeedClassHeap()
{
    std::cerr << "Allocating Memory (Object Size : " << sizeof(MyTestClass) << ")..." ;

    for(unsigned int j = 0; j < TestCount; j++)
    {
        MyTestClass *ptrTestClass = new MyTestClass ;
        delete ptrTestClass ;
    }

    std::cerr << "OK" << std::endl ;


}

/******************
TestAllocationSpeedArrayMemPool
******************/
void TestAllocationSpeedArrayMemPool()
{
    std::cerr << "Allocating Memory (Object Size : " << ArraySize << ")..." ;
    for(unsigned int j = 0; j < TestCount; j++)
    {
        char *ptrArray = (char *) g_ptrMemPool->GetMemory(ArraySize)  ;
        g_ptrMemPool->FreeMemory(ptrArray, ArraySize) ;
    }

    std::cerr << "OK" << std::endl ;

}

/******************
TestAllocationSpeedArrayHeap
******************/
void TestAllocationSpeedArrayHeap()
{
    std::cerr << "Allocating Memory (Object Size : " << ArraySize << ")..." ;

    for(unsigned int j = 0; j < TestCount; j++)
    {
        char *ptrArray = (char *) malloc(ArraySize)  ;
        free(ptrArray) ;
    }

    std::cerr << "OK" << std::endl ;

}



/******************
WriteMemoryDumpToFile
******************/
void WriteMemoryDumpToFile()
{
    std::cerr << "Writing MemoryDump to File..." ;
    g_ptrMemPool->WriteMemoryDumpToFile("MemoryDump.bin") ;
    std::cerr << "OK" << std::endl ;
}

/******************
main
******************/
int main(int argc, char *argv[])
{
    /**std::cout << "MemoryPool Program started..." << std::endl ;
    CreateGlobalMemPool() ;

    TestAllocationSpeedArrayMemPool() ;
    TestAllocationSpeedArrayHeap() ;

    TestAllocationSpeedClassMemPool() ;
    TestAllocationSpeedClassHeap() ;

**/
    CreateGlobalMemPool() ;
    MyTestClass *ptrTestClass = new MyTestClass ;
    g_ptrMemPool->WriteMemoryDumpToFile("MemoryDump.bin");
    std::cout << "MemoryPool Program finished..." << std::endl ;
    system("PAUSE") ;
    return 0 ;
}

