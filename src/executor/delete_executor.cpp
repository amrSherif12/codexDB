#include "../../include/executor/delete_executor.h"

DeleteExecutor::DeleteExecutor(std::unique_ptr<Executor> child, ScanExecutor *scan_executor,
                               BTree *btree) : child(std::move(child)), scan_executor(scan_executor), btree(btree) {
}

bool DeleteExecutor::init() {
    return child->init();
}

std::optional<Tuple> DeleteExecutor::next() {
    std::optional<Tuple> tuple = child->next();
    if (!tuple.has_value()) return std::nullopt;
    btree->remove(tuple->row.id);
    scan_executor->decrement_cursor();
    return tuple;
}
