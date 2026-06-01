#include "../include/cursor.h"

#include <iostream>

Cursor::Cursor(Table *table, uint32_t page_num, uint16_t slot_index) : table(table), page_num(page_num),
                                                                       slot_index(slot_index) {
}

bool Cursor::end_of_table() {
    return page_num == 0;
}

char *Cursor::value() {
    if (end_of_table()) return nullptr;
    PageGuard page = table->get_page(page_num);
    return page->get_record(slot_index);
}

void Cursor::advance() {
    if (end_of_table()) return;
    PageGuard page = table->get_page(page_num);
    slot_index++;
    while (true) {
        if (slot_index < page->header->slot_cnt) {
            if (page->get_record(slot_index)) return;
            slot_index++;
        } else {
            page_num = page->header->next_page_id;
            if (end_of_table()) return;
            slot_index = 0;
            page = table->get_page(page_num);
        }
    }
}
