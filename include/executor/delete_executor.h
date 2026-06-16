#ifndef DELETE_EXECUTOR_H
#define DELETE_EXECUTOR_H

#include "executor.h"
#include "scan_executor.h"
#include "../btree.h"

class DeleteExecutor : public Executor {
    BTree *btree;
    std::unique_ptr<Executor> child;
    ScanExecutor *scan_executor;
public:
    DeleteExecutor(std::unique_ptr<Executor> child, ScanExecutor *scan_executor, BTree *btree);

    bool init() override;
    std::optional<Tuple> next() override;
};

#endif //DELETE_EXECUTOR_H
