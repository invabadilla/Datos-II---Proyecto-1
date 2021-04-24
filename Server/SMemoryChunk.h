//
// Created by usuario on 4/10/21.
//

#ifndef SERVER_SMEMORYCHUNK_H
#define SERVER_SMEMORYCHUNK_H

#include "IMemoryBlock.h"
#include "Variable_struct.h"
#include <vector>
namespace MemPool {

    /**
     * Struct para la creacion de cada Chunk del Pool de memoria
     */
    typedef struct SMemoryChunk {
        std::string type;                   //Tipo de la variable almacenada
        std::string name;                   //Nombre de la variable
        int counter;                        //Contador de referencias
        bool isReference;                   //Bool para saber si es una referencia
        bool isscope;                       //bool para saber si esta en un Scope
        int deap;                           //Contador de la profundidad de los Scope
        SMemoryChunk *reference;            //Puntero a la direccion que almacena cuando la variable es Reference
        std::vector <Variable_struct> mstruct; //Vector para almacenar las referencias hacia las variables del Struct
        bool isstruct;                      //Bool de si la variable es de tipo Struct
        TByte *Data;                        //El dato actual
        size_t DataSize;                    //Size del bloque de datos
        size_t UsedSize;                    //Size usado acutal
        bool IsAllocationChunk;             //True cuando el MemoryChunks apunta a un Data-block libre
        SMemoryChunk *Next;                 //Puntero al proximo MemoryChunk
    } SMemoryChunk;
}
#endif //SERVER_SMEMORYCHUNK_H
