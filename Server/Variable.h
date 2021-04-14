//
// Created by ingrid on 4/13/21.
//

#ifndef SERVER_VARIABLE_H
#define SERVER_VARIABLE_H

#include <string>

using namespace std;
class Variable {
public:
    Variable(string type_, string name_, int *ptr_, int counter_);
    string type;
    string name;
    int *ptr;
    int counter;
};


#endif //SERVER_VARIABLE_H
