#include "../include/table.h"

Table::Table(const std::string &file_name, BufferPoolManager *bpm) {
    this->bpm = bpm;
    if (bpm->is_new_file()) {
        bpm->init_file();
    }
}

Table::~Table() {
    flush_all();
}

void Table::flush_all() {
    bpm->flush_all_pages();
}

PageGuard Table::get_page(uint32_t page_id) {
    return {this, bpm->fetch_page(page_id), false, page_id};
}

PageGuard Table::allocate_new_page(PageType type) {
    SuperBlockGuard superblock = get_superblock();
    uint32_t new_id;
    PageGuard p{nullptr, nullptr, false, 0};

    if (superblock->first_free_page == 0) {
        new_id = superblock->next_auto_increment_page_id;
        superblock->next_auto_increment_page_id++;
    } else {
        new_id = superblock->first_free_page;
        p = get_page(new_id);
        superblock->first_free_page = p->header->next_page_id;
    }

    Page *page = bpm->allocate_page(new_id);
    page->init_new_page(type, new_id);
    superblock.mark_dirty();
    return {this, page, true, new_id};
}

void Table::free_page(uint32_t page_id) {
    if (page_id == 0) return;

    SuperBlockGuard superblock = get_superblock();
    PageGuard page = get_page(page_id);

    if (page->header->type == FREE_NODE) return;

    page->init_new_page(FREE_NODE, page_id);
    page->header->next_page_id = superblock->first_free_page;
    superblock->first_free_page = page_id;

    superblock.mark_dirty();
    page.mark_dirty();
}

SuperBlockGuard Table::get_superblock() {
    return SuperBlockGuard(get_page(0));
}


uint32_t Table::increment_superblock_row_id() {
    SuperBlockGuard superblock = get_superblock();
    superblock->next_auto_increment_row_id++;
    superblock.mark_dirty();
    return superblock->next_auto_increment_row_id - 1;
}

void Table::unpin_page(uint32_t page_id, bool is_dirty) {
    bpm->unpin_page(page_id, is_dirty);
}
