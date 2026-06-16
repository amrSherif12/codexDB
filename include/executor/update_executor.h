#ifndef UPDATE_EXECUTOR_H
#define UPDATE_EXECUTOR_H

#include "executor.h"
#include "../btree.h"

class UpdateExecutor : public Executor {
    BTree *btree;
    std::unique_ptr<Executor> child;
    Tuple new_tuple;
public:
    UpdateExecutor(std::unique_ptr<Executor> child, Tuple &new_tuple, BTree *btree);

    bool init() override;
    std::optional<Tuple> next() override;
};

#endif //UPDATE_EXECUTOR_H
