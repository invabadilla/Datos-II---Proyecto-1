//
// Created by ingrid on 4/13/21.
//

#include "Variable.h"
template <class T>

Variable<T>::Variable(string type_, string name_, T *ptr_, int counter_) {
    Variable::type = type_;
    Variable::name = name_;
    Variable::ptr = ptr_;
    Variable::counter = counter_;
}

