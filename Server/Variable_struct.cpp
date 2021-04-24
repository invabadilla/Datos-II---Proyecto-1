//
// Created by usuario on 4/21/21.
//

#include "Variable_struct.h"
#include <string>

/**
 * Constructor para la creacion de tipo variable_struct que administra los
 * Struct creadas en el Pool de memoria
 * @param name_ Nombre del Struct
 */
Variable_struct::Variable_struct(std::string name_) {
    Variable_struct::name = name_;
}