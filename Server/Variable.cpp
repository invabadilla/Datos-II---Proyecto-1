//
// Created by ingrid on 4/13/21.
//

#include "Variable.h"
#include "SMemoryChunk.h"

/**
 * Constructor para la creacion de tipo variable que administra las
 * variables creadas en el Pool de memoria
 * @param name_ Nombre de la variable
 * @param ptr_ Puntero de referencia
 */
Variable::Variable(string name_, MemPool::SMemoryChunk *ptr_) {
    Variable::name = name_;
    Variable::ptr = ptr_;
}
