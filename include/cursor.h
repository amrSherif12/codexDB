#ifndef CURSOR_H
#define CURSOR_H

#include "table.h"

class Cursor {
    Table *table;

public:
    uint32_t page_num;
    uint16_t slot_index;

    Cursor(Table *table, uint32_t page_num, uint16_t slot_index);

    bool end_of_table();

    char *value();

    void advance();
};

#endif //CURSOR_H
