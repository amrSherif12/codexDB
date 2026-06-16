#ifndef INSERT_EXECUTOR_H
#define INSERT_EXECUTOR_H
#include "executor.h"
#include "../btree.h"

class InsertExecutor : public Executor {
    Tuple tuple;
    BTree *btree;

public:
    InsertExecutor(Tuple &tuple, BTree *btree);

    bool init() override;
    std::optional<Tuple> next() override;
};

#endif //INSERT_EXECUTOR_H


