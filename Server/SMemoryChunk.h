//
// Created by usuario on 4/10/21.
//

#ifndef SERVER_SMEMORYCHUNK_H
#define SERVER_SMEMORYCHUNK_H

#include "IMemoryBlock.h"

namespace MemPool {

    typedef struct SMemoryChunk {
        std::string type;         //TIpo de la variable almacenada
        std::string name;         //Nombre de la variable
        int counter;              //contador de referencias
        bool isReference;
        SMemoryChunk *reference ;
        TByte *Data;              //El dato actual
        size_t DataSize;          //Tamano del bloque de datos
        size_t UsedSize;          //Tamano usado acutal
        bool IsAllocationChunk;   //True cuando el MemoryChunks apunta a un Data-block libre
        SMemoryChunk *Next;       //Puntero al proximo MemoryChunk
    } SMemoryChunk;
}
#endif //SERVER_SMEMORYCHUNK_H
