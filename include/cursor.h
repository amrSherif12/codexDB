#ifndef CURSOR_H
#define CURSOR_H

#include <cstdint>

class BTree;

class Cursor {
    BTree *btree;
public:
    uint32_t page_num;
    uint16_t slot_index;

    Cursor(BTree *btree, uint32_t page_num, uint16_t slot_index);

    bool end_of_table();

    char *value();

    void advance();
};

#endif //CURSOR_H
