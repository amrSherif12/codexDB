#include "../include/table.h"
#include <cstring>
#include <iostream>

Table::Table(const std::string &file_name) : pager(file_name), pages{} {
    if (pager.file_length == 0) {
        char sb_buffer[PAGE_SIZE] = {0};
        SuperBlock *super_block = reinterpret_cast<SuperBlock *>(sb_buffer);
        super_block->root_page_id = 1;
        super_block->next_auto_increment_id = 0;
        pager.write_page(0, sb_buffer);
        char p_buffer[PAGE_SIZE] = {0};
        Page first_page(p_buffer);
        first_page.init_new_page(LEAF_NODE, 1);
        pager.write_page(1, p_buffer);
    }
}

Table::~Table() {
    save_pages();
}

void Table::save_pages() {
    for (int i = 0; i < MAX_PAGES; i++) {
        if (pages[i] != nullptr && dirty_bits[i]) {
            pager.write_page(i, pages[i]->data);
            delete[] pages[i]->data;
            delete pages[i];
        }
    }
}


Page *Table::get_page(uint32_t page_id) {
    if (pages[page_id] == nullptr) {
        char *data = new char[PAGE_SIZE];
        pages[page_id] = new Page(data);
        if (page_id * PAGE_SIZE < pager.file_length) {
            pager.read_page(page_id, pages[page_id]->data);
        }
    }
    return pages[page_id];
}

Page *Table::allocate_new_page(PageType type) {
    SuperBlock *superblock = get_superblock();
    uint32_t new_id;

    if (superblock->first_free_page == 0) {
        new_id = pager.file_length / PAGE_SIZE;
        pager.file_length += PAGE_SIZE;
    } else {
        new_id = superblock->first_free_page;
        Page *p = get_page(new_id);
        superblock->first_free_page = p->header->next_page_id;
    }

    Page *page = get_page(new_id);
    page->init_new_page(type, new_id);

    dirty_bits.set(0);
    return page;
}

void Table::free_page(uint32_t page_id) {
    if (page_id == 0) return;

    SuperBlock *superblock = get_superblock();
    Page *page = get_page(page_id);

    if (page->header->type == FREE_NODE) return;

    page->init_new_page(FREE_NODE, page_id);
    page->header->next_page_id = superblock->first_free_page;
    superblock->first_free_page = page_id;

    dirty_bits.set(page_id);
    dirty_bits.set(0);
}

SuperBlock *Table::get_superblock() {
    Page *superblock = get_page(0);
    return reinterpret_cast<SuperBlock *>(superblock->data);
}

void Table::increment_superblock_id() {
    get_superblock()->next_auto_increment_id++;
    dirty_bits.set(0);
}
