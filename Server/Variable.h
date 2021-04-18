//
// Created by ingrid on 4/13/21.
//

#ifndef SERVER_VARIABLE_H
#define SERVER_VARIABLE_H

#include <string>
#include "SMemoryChunk.h"

using namespace std;
/**template <class T>

class Variable {
public:
    Variable(string type_, string name_, T *ptr_, int counter_, bool reference_);
    string type;
    string name;
    T *ptr;
    int counter;
    bool reference = false;
};**/

class Variable {
public:
    Variable(string name_, MemPool::SMemoryChunk *ptr_);
    string name;
    MemPool::SMemoryChunk *ptr;
};


#endif //SERVER_VARIABLE_H
