//
// Created by ingrid on 4/13/21.
//

#ifndef SERVER_VARIABLE_H
#define SERVER_VARIABLE_H

#include <string>
#include "SMemoryChunk.h"

using namespace std;

/**
 * Clase para la creacion de Variable que administra todas las
 * variables creadas en el Pool de memoria
 */
class Variable {
public:
    Variable(string name_, MemPool::SMemoryChunk *ptr_);
    string name; //Nombre de la variable
    MemPool::SMemoryChunk *ptr; //Puntero de referencia hacia la variable
};

#endif //SERVER_VARIABLE_H
