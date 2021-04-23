//
// Created by ingrid on 4/13/21.
//

#include "Variable.h"
#include "SMemoryChunk.h"

Variable::Variable(string name_, MemPool::SMemoryChunk *ptr_) {
    Variable::name = name_;
    Variable::ptr = ptr_;
}
