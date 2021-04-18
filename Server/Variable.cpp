//
// Created by ingrid on 4/13/21.
//

#include "Variable.h"
#include "SMemoryChunk.h"
//template <class T>

//Variable<T>::Variable(string type_, string name_, T *ptr_, int counter_, bool reference_) {
/**Variable::type = type_;
Variable::name = name_;
Variable::ptr = ptr_;
Variable::counter = counter_;
Variable::reference = reference_;**/

Variable::Variable(string name_, MemPool::SMemoryChunk *ptr_) {
    Variable::name = name_;
    Variable::ptr = ptr_;
}


