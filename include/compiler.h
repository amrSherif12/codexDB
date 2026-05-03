
#ifndef COMPILER_H
#define COMPILER_H
#include <string>

#include "statement.h"

PrepareResult prepare_statement(const std::string &input_buffer, Statement *statement);

#endif //COMPILER_H
