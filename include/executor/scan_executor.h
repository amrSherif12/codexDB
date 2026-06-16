
#ifndef SCAN_EXECUTOR_H
#define SCAN_EXECUTOR_H
#include "executor.h"
#include "../btree.h"

#include <memory>
#include <optional>

class ScanExecutor : public Executor {
    BTree *btree;
    std::unique_ptr<Cursor> cursor;
public:
    explicit ScanExecutor(BTree *btree);

    bool init() override;
    std::optional<Tuple> next() override;
    void decrement_cursor();
};

#endif //SCAN_EXECUTOR_H
