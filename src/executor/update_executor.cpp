#include "../../include/executor/update_executor.h"

UpdateExecutor::UpdateExecutor(std::unique_ptr<Executor> child, Tuple &new_tuple, BTree *btree) : child(std::move(child)), new_tuple(new_tuple), btree(btree) {

}

bool UpdateExecutor::init() {
    return child->init();
}


std::optional<Tuple> UpdateExecutor::next() {
    std::optional<Tuple> tuple = child->next();
    if (!tuple.has_value()) return std::nullopt;
    new_tuple.row.id = tuple->row.id;
    char dest[new_tuple.row.get_size()];
    new_tuple.row.serialize(dest);
    btree->update(new_tuple.row.id, dest, new_tuple.row.get_size());
    return tuple;
}
