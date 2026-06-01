#ifndef TABLE_H
#define TABLE_H

#include "buffer_pool_manager.h"
#include "page.h"
#include "page_guard.h"


class Table {
    BufferPoolManager *bpm;

public:
    Table(const std::string &file_name, BufferPoolManager *bpm);

    ~Table();

    void flush_all();

    PageGuard get_page(uint32_t page_id);

    PageGuard allocate_new_page(PageType type);

    void free_page(uint32_t page_id);

    SuperBlockGuard get_superblock();

    int increment_superblock_row_id(SuperBlockGuard &sb_page);

    void unpin_page(uint32_t page_id, bool is_dirty);
};

#endif //TABLE_H
