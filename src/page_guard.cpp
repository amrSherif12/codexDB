#include "../include/page_guard.h"
#include "../include/table.h"

PageGuard::PageGuard(Table *table, Page *page, bool is_dirty, uint32_t page_id) : table(table), page(page),
    is_dirty(is_dirty), page_id(page_id) {
}

PageGuard::~PageGuard() {
    if (table && page) table->unpin_page(page_id, is_dirty);
}

PageGuard &PageGuard::operator=(PageGuard &&other) noexcept {
    if (this != &other) {
        if (table && page) table->unpin_page(page_id, is_dirty);

        this->table = other.table;
        this->page = other.page;
        this->is_dirty = other.is_dirty;
        this->page_id = other.page_id;

        other.page = nullptr;
    }
    return *this;
}

PageGuard::PageGuard(PageGuard &&other) noexcept : table(other.table), page(other.page), is_dirty(other.is_dirty),
                                                   page_id(other.page_id) {
    other.page = nullptr;
}

Page *PageGuard::operator->() { return page; }

Page &PageGuard::operator*() { return *page; }

Page *PageGuard::get() { return page; }

void PageGuard::mark_dirty() { is_dirty = true; }

SuperBlockGuard::SuperBlockGuard(PageGuard &&guard_param) : guard(std::move(guard_param)) {
    sb = reinterpret_cast<SuperBlock *>(this->guard->data);
}

SuperBlock *SuperBlockGuard::operator->() { return sb; }

SuperBlock &SuperBlockGuard::operator*() { return *sb; }

void SuperBlockGuard::mark_dirty() { guard.mark_dirty(); }
