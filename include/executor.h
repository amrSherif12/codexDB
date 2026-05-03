
#ifndef EXECUTOR_H
#define EXECUTOR_H
#include "btree.h"
#include "statement.h"

ExecuteResult execute_statement(Statement *statement, BTree *btree);

#endif //EXECUTOR_H
