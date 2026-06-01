#ifndef PAGE_GUARD_H
#define PAGE_GUARD_H

#include <algorithm>

#include "page.h"

class Table;
class Page;

class PageGuard {
    Table *table;
    Page *page;
    bool is_dirty;
    uint32_t page_id;

public:
    PageGuard(Table *table, Page *page, bool is_dirty, uint32_t page_id);

    ~PageGuard();

    PageGuard(const PageGuard &) = delete;

    PageGuard &operator=(PageGuard &&) noexcept;

    PageGuard(PageGuard &&other) noexcept;

    Page *operator->();

    Page &operator*();

    Page *get();

    void mark_dirty();
};

class SuperBlockGuard {
    PageGuard guard;
    SuperBlock *sb;

public:
    SuperBlockGuard(PageGuard &&guard);

    SuperBlock *operator->();

    SuperBlock &operator*();

    void mark_dirty();
};

#endif //PAGE_GUARD_H
