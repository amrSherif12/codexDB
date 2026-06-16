#include "../../include/executor/scan_executor.h"

#include <iostream>

ScanExecutor::ScanExecutor(BTree *btree) : btree(btree), cursor(nullptr) {
}

bool ScanExecutor::init() {
    SearchResult search_result = btree->begin();
    if (!search_result.is_found) return false;
    cursor = std::make_unique<Cursor>(search_result.cursor);
    return true;
}

std::optional<Tuple> ScanExecutor::next() {
    if (cursor->end_of_table()) return std::nullopt;
    Row row;
    row.deserialize(cursor->value());
    Tuple tuple{row};
    cursor->advance();
    return tuple;
}

void ScanExecutor::decrement_cursor() { cursor->slot_index--; }
