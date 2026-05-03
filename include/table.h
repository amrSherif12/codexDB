#ifndef TABLE_H
#define TABLE_H

#include <bitset>
#include "pager.h"
#include "page.h"

constexpr int MAX_PAGES = 500;

class Table {
    Pager pager;
    Page *pages[MAX_PAGES];

public:
    std::bitset<MAX_PAGES> dirty_bits;

    Table(const std::string &file_name);

    ~Table();

    void save_pages();

    Page *get_page(uint32_t page_id);

    Page *allocate_new_page(PageType type);

    void free_page(uint32_t page_id);

    SuperBlock *get_superblock();

    void increment_superblock_id();
};

#endif //TABLE_H
