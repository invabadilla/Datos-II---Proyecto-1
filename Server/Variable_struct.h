//
// Created by usuario on 4/21/21.
//

#ifndef SERVER_VARIABLE_STRUCT_H
#define SERVER_VARIABLE_STRUCT_H


#include <string>

/**
 * Constructor para la creacion de tipo variable_struct que administra los
 * Struct creadas en el Pool de memoria
 */
class Variable_struct {
    public:
    Variable_struct(std::string name_);

    std::string name; //Nombre del Struct

};

#endif //SERVER_VARIABLE_STRUCT_H
