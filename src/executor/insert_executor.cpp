#include <utility>

#include "../../include/executor/insert_executor.h"
#include "../../include/table.h"

InsertExecutor::InsertExecutor(Tuple &tuple, BTree *btree) : tuple(tuple), btree(btree) {
}


bool InsertExecutor::init() {
    return true;
}

std::optional<Tuple> InsertExecutor::next() {
    uint32_t id = btree->table->increment_superblock_row_id();
    char dest[tuple.row.get_size()];

    tuple.row.id = id;
    tuple.row.serialize(dest);
    btree->insert(id, dest, tuple.row.get_size());
    return std::nullopt;
}
