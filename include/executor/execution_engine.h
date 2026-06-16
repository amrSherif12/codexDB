#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H
#include <memory>
#include "executor.h"
#include "../btree.h"

class ExecutionEngine {
    std::unique_ptr<Executor> build_pipeline(Statement &statement, BTree *btree);
public:
    void execute(Statement &statement, BTree *btree);
};

#endif //EXECUTION_ENGINE_H
