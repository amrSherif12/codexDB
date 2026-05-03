#include "../include/btree.h"

#include <iostream>
#include <vector>


BTree::BTree(Table *table) : table(table) {
}

InternalNodeCell *BTree::get_internal_cell(Page *page, uint16_t cell_num) {
    return reinterpret_cast<InternalNodeCell *>(page->data + sizeof(PageHeader) + cell_num * sizeof(InternalNodeCell));
}

bool BTree::insert_internal_cell(Page *page, uint32_t key, uint32_t left_child, uint32_t right_child) {
    if (sizeof(PageHeader) + sizeof(InternalNodeCell) * (page->header->slot_cnt + 1) > PAGE_SIZE) return false;
    uint16_t insert_index = page->header->slot_cnt;
    int l = 0, r = page->header->slot_cnt - 1, m;
    while (l <= r) {
        m = l + (r - l) / 2;
        InternalNodeCell *cell = get_internal_cell(page, m);
        if (key < cell->key) {
            insert_index = m;
            r = m - 1;
        } else l = m + 1;
    }

    if (insert_index < page->header->slot_cnt) {
        InternalNodeCell *insert_pos = get_internal_cell(page, insert_index);
        InternalNodeCell *shift_pos = get_internal_cell(page, insert_index + 1);
        memmove(shift_pos, insert_pos,
                (page->header->slot_cnt - insert_index) * sizeof(InternalNodeCell));
    }

    InternalNodeCell *new_cell = get_internal_cell(page, insert_index);
    new_cell->child_page = left_child;
    new_cell->key = key;
    page->header->slot_cnt++;

    if (insert_index + 1 < page->header->slot_cnt) {
        InternalNodeCell *next_cell = get_internal_cell(page, insert_index + 1);
        next_cell->child_page = right_child;
    } else {
        page->header->rightmost_child_id = right_child;
    }
    return true;
}

void BTree::remove_internal_cell(Page *parent, uint32_t child_to_delete) {
    uint16_t index_to_remove = -1;

    if (parent->header->rightmost_child_id == child_to_delete) {
        InternalNodeCell *cell = get_internal_cell(parent, parent->header->slot_cnt - 1);
        parent->header->rightmost_child_id = cell->child_page;
        index_to_remove = parent->header->slot_cnt - 1;
    } else {
        for (uint16_t i = 0; i < parent->header->slot_cnt; i++) {
            InternalNodeCell *cell = get_internal_cell(parent, i);
            if (cell->child_page == child_to_delete) {
                index_to_remove = i;
                break;
            }
        }

        if (index_to_remove > 0 && index_to_remove < parent->header->slot_cnt) {
            InternalNodeCell *prev_cell = get_internal_cell(parent, index_to_remove - 1);
            InternalNodeCell *curr_cell = get_internal_cell(parent, index_to_remove);
            prev_cell->key = curr_cell->key;
        }
    }

    if (index_to_remove < parent->header->slot_cnt - 1) {
        InternalNodeCell *dst = get_internal_cell(parent, index_to_remove);
        InternalNodeCell *src = get_internal_cell(parent, index_to_remove + 1);
        memmove(dst, src, (parent->header->slot_cnt - index_to_remove - 1) * sizeof(InternalNodeCell));
    }

    parent->header->slot_cnt--;

    if (parent->header->parent_id != 0 && parent->is_underflowed()) {
        balance_internal(parent);
    } else if (parent->header->parent_id == 0 && parent->header->slot_cnt == 0) {
        table->get_superblock()->root_page_id = parent->header->rightmost_child_id;
        Page *new_root = table->get_page(parent->header->rightmost_child_id);
        new_root->header->parent_id = 0;
        table->free_page(parent->header->page_id);
    }
}

uint32_t BTree::find_leaf_page(uint32_t key, uint32_t page_id) {
    Page *page;
    if (page_id == 0) page = table->get_page(table->get_superblock()->root_page_id);
    else page = table->get_page(page_id);
    if (page->header->type == LEAF_NODE) return page->header->page_id;

    uint16_t child = page->header->rightmost_child_id;
    int l = 0, r = page->header->slot_cnt - 1, m;
    while (l <= r) {
        m = l + (r - l) / 2;
        InternalNodeCell *cell = get_internal_cell(page, m);
        if (key < cell->key) {
            child = cell->child_page;
            r = m - 1;
        } else l = m + 1;
    }

    return find_leaf_page(key, child);
}

SplitResult BTree::split_leaf_node(Cursor *cursor) {
    Page *old_page = table->get_page(cursor->page_num);
    Page *new_page = table->allocate_new_page(LEAF_NODE);

    uint16_t original_slot_cnt = old_page->header->slot_cnt;
    uint16_t split_index = original_slot_cnt / 2;

    for (uint16_t i = split_index; i < original_slot_cnt; i++) {
        Slot *slot = old_page->get_slot(i);
        char *record = old_page->get_record(i);

        new_page->insert_record(record, slot->size, i - split_index);
        old_page->header->garbage_cnt += slot->size;
    }

    old_page->header->slot_cnt = split_index;
    old_page->header->lower_offset = sizeof(PageHeader) + sizeof(Slot) * split_index;
    old_page->defragment();

    new_page->header->next_page_id = old_page->header->next_page_id;
    new_page->header->prev_page_id = old_page->header->page_id;
    old_page->header->next_page_id = new_page->header->page_id;

    if (new_page->header->next_page_id != 0) {
        Page *next_page = table->get_page(new_page->header->next_page_id);
        next_page->header->prev_page_id = new_page->header->page_id;
    }
    new_page->header->parent_id = old_page->header->parent_id;

    char *mid_record = new_page->get_record(0);
    uint32_t mid_key;
    memcpy(&mid_key, mid_record, sizeof(uint32_t));

    if (new_page->header->parent_id == 0) {
        Page *new_root = table->allocate_new_page(INTERNAL_NODE);

        InternalNodeCell *first_cell = get_internal_cell(new_root, 0);
        first_cell->child_page = old_page->header->page_id;
        first_cell->key = mid_key;

        new_root->header->rightmost_child_id = new_page->header->page_id;
        new_root->header->slot_cnt = 1;

        old_page->header->parent_id = new_root->header->page_id;
        new_page->header->parent_id = new_root->header->page_id;

        InternalNodeCell *first_ = get_internal_cell(new_root, 0);

        table->get_superblock()->root_page_id = new_root->header->page_id;
    } else {
        Page *parent_page = table->get_page(new_page->header->parent_id);
        if (!insert_internal_cell(parent_page, mid_key, old_page->header->page_id, new_page->header->page_id)) {
            SplitResult split_result = split_internal_node(parent_page->header->page_id);

            if (mid_key < split_result.pushed_up_key) {
                insert_internal_cell(parent_page, mid_key, old_page->header->page_id, new_page->header->page_id);
            } else {
                Page *new_parent = table->get_page(split_result.new_page_id);
                insert_internal_cell(new_parent, mid_key, old_page->header->page_id, new_page->header->page_id);
            }
        }
    }
    return {new_page->header->page_id, mid_key};
}

SplitResult BTree::split_internal_node(uint32_t internal_page_id) {
    Page *old_page = table->get_page(internal_page_id);
    Page *new_page = table->allocate_new_page(INTERNAL_NODE);

    uint16_t original_slot_cnt = old_page->header->slot_cnt;
    uint16_t split_index = original_slot_cnt / 2;
    InternalNodeCell *mid_cell = get_internal_cell(old_page, split_index);

    uint16_t cells_to_move = original_slot_cnt - split_index - 1;
    InternalNodeCell *src = get_internal_cell(old_page, split_index + 1);
    InternalNodeCell *dst = get_internal_cell(new_page, 0);
    memcpy(dst, src, cells_to_move * sizeof(InternalNodeCell));

    new_page->header->rightmost_child_id = old_page->header->rightmost_child_id;
    old_page->header->rightmost_child_id = mid_cell->child_page;

    old_page->header->slot_cnt = split_index;
    new_page->header->slot_cnt = cells_to_move;

    for (uint16_t i = 0; i < new_page->header->slot_cnt; i++) {
        InternalNodeCell *cell = get_internal_cell(new_page, i);
        Page *child = table->get_page(cell->child_page);
        child->header->parent_id = new_page->header->page_id;
    }

    Page *right_most_page = table->get_page(new_page->header->rightmost_child_id);
    right_most_page->header->parent_id = new_page->header->page_id;

    new_page->header->parent_id = old_page->header->page_id;

    if (old_page->header->parent_id == 0) {
        Page *new_root = table->allocate_new_page(INTERNAL_NODE);

        InternalNodeCell *first_cell = get_internal_cell(new_root, 0);
        first_cell->child_page = old_page->header->page_id;
        first_cell->key = mid_cell->key;
        new_root->header->rightmost_child_id = new_page->header->page_id;

        old_page->header->parent_id = new_root->header->page_id;
        new_page->header->parent_id = new_root->header->page_id;

        new_root->header->slot_cnt = 1;
        table->get_superblock()->root_page_id = new_root->header->page_id;
    } else {
        Page *parent_page = table->get_page(old_page->header->parent_id);
        if (!insert_internal_cell(parent_page, mid_cell->key, old_page->header->page_id, new_page->header->page_id)) {
            SplitResult split_result = split_internal_node(parent_page->header->page_id);

            if (mid_cell->key < split_result.pushed_up_key) {
                insert_internal_cell(parent_page, mid_cell->key, old_page->header->page_id, new_page->header->page_id);
            } else {
                Page *new_parent = table->get_page(split_result.new_page_id);
                insert_internal_cell(new_parent, mid_cell->key, old_page->header->page_id, new_page->header->page_id);
            }
        }
    }
    return {new_page->header->page_id, mid_cell->key};
}

void BTree::merge_internal(Page *left, Page *right, Page *parent, uint16_t parent_index) {
    InternalNodeCell *parent_cell = get_internal_cell(parent, parent_index);

    InternalNodeCell *join_cell = get_internal_cell(left, left->header->slot_cnt);
    join_cell->key = parent_cell->key;
    join_cell->child_page = left->header->rightmost_child_id;
    left->header->slot_cnt++;

    InternalNodeCell *right_cells = get_internal_cell(right, 0);
    InternalNodeCell *left_dest = get_internal_cell(left, left->header->slot_cnt);
    memcpy(left_dest, right_cells, right->header->slot_cnt * sizeof(InternalNodeCell));

    left->header->rightmost_child_id = right->header->rightmost_child_id;

    uint16_t start_update_idx = left->header->slot_cnt;
    left->header->slot_cnt += right->header->slot_cnt;

    for (uint16_t i = start_update_idx; i < left->header->slot_cnt; i++) {
        InternalNodeCell *cell = get_internal_cell(left, i);
        Page *child = table->get_page(cell->child_page);
        child->header->parent_id = left->header->page_id;
    }

    left->header->rightmost_child_id = right->header->rightmost_child_id;
    remove_internal_cell(parent, right->header->page_id);

    table->free_page(right->header->page_id);
}

// if from_beg = true then we borrow from beginning of the src to the end of dst

void BTree::borrow_internal(Page *src, Page *dst, Page *parent, uint16_t parent_index, bool from_beg) {
    InternalNodeCell *parent_cell = get_internal_cell(parent, parent_index);

    while (dst->is_underflowed()) {
        if (from_beg) {
            InternalNodeCell *new_dst_cell = get_internal_cell(dst, dst->header->slot_cnt);

            new_dst_cell->key = parent_cell->key;
            new_dst_cell->child_page = dst->header->rightmost_child_id;

            InternalNodeCell *src_first = get_internal_cell(src, 0);
            dst->header->rightmost_child_id = src_first->child_page;
            parent_cell->key = src_first->key;

            memmove(src_first, get_internal_cell(src, 1), (src->header->slot_cnt - 1) * sizeof(InternalNodeCell));

            Page *moved_child = table->get_page(dst->header->rightmost_child_id);
            moved_child->header->parent_id = dst->header->page_id;
        } else {
            memmove(get_internal_cell(dst, 1), get_internal_cell(dst, 0),
                    dst->header->slot_cnt * sizeof(InternalNodeCell));

            InternalNodeCell *dst_first = get_internal_cell(dst, 0);
            dst_first->key = parent_cell->key;
            dst_first->child_page = src->header->rightmost_child_id;

            InternalNodeCell *src_last = get_internal_cell(src, src->header->slot_cnt - 1);
            parent_cell->key = src_last->key;
            src->header->rightmost_child_id = src_last->child_page;

            Page *moved_child = table->get_page(dst_first->child_page);
            moved_child->header->parent_id = dst->header->page_id;
        }
        src->header->slot_cnt--;
        dst->header->slot_cnt++;
    }
}

uint16_t BTree::get_child_index(Page *parent, uint16_t child_page_id) {
    for (int i = 0; i < parent->header->slot_cnt; i++) {
        InternalNodeCell *cell = get_internal_cell(parent, i);
        if (cell->child_page == child_page_id) return i;
    }
    if (parent->header->rightmost_child_id == child_page_id) return parent->header->slot_cnt;
    return -1;
}

void BTree::balance_leaf(Page *leaf_page) {
    uint32_t required_page_size = PAGE_SIZE - sizeof(PageHeader);
    uint32_t leaf_page_size = leaf_page->leaf_page_size();

    uint32_t left_page_id = leaf_page->header->prev_page_id;
    Page *left_page = table->get_page(left_page_id);
    if (left_page_id != 0) {
        uint32_t left_page_size = left_page->leaf_page_size();
        if (leaf_page->header->parent_id == left_page->header->parent_id && leaf_page_size + left_page_size >=
            required_page_size) {
            borrow_leaf(left_page, leaf_page, false);
            return;
        }
    }

    uint32_t right_page_id = leaf_page->header->next_page_id;
    Page *right_page = table->get_page(right_page_id);
    if (right_page_id != 0) {
        uint32_t right_page_size = right_page->leaf_page_size();
        if (leaf_page->header->parent_id == right_page->header->parent_id && leaf_page_size + right_page_size >=
            required_page_size) {
            borrow_leaf(right_page, leaf_page, true);
            return;
        }
    }

    if (leaf_page->header->parent_id == left_page->header->parent_id) merge_leaf(left_page, leaf_page);
    else merge_leaf(leaf_page, right_page);
}

void BTree::balance_internal(Page *internal_page) {
    Page *parent = table->get_page(internal_page->header->parent_id);
    Page *left_page;
    Page *right_page;

    uint16_t my_index = get_child_index(parent, internal_page->header->page_id);
    uint32_t max_internal_cells = (PAGE_SIZE - sizeof(PageHeader)) / sizeof(InternalNodeCell);

    if (my_index > 0) {
        uint16_t left_page_index = my_index - 1;
        uint32_t left_page_id = get_internal_cell(parent, left_page_index)->child_page;
        left_page = table->get_page(left_page_id);

        if (internal_page->header->slot_cnt + left_page->header->slot_cnt + 1 >= max_internal_cells) {
            borrow_internal(left_page, internal_page, parent, left_page_index, false);
            return;
        }
    }

    if (my_index < parent->header->slot_cnt) {
        uint16_t right_page_index = my_index;
        uint32_t right_page_id;
        if (right_page_index + 1 == parent->header->slot_cnt) {
            right_page_id = parent->header->rightmost_child_id;
        } else {
            right_page_id = get_internal_cell(parent, right_page_index + 1)->child_page;
        }
        right_page = table->get_page(right_page_id);

        if (internal_page->header->slot_cnt + right_page->header->slot_cnt + 1 >= max_internal_cells) {
            borrow_internal(right_page, internal_page, parent, right_page_index, true);
            return;
        }
    }

    if (my_index > 0) {
        merge_internal(left_page, internal_page, parent, my_index - 1);
    } else {
        merge_internal(internal_page, right_page, parent, my_index);
    }
}

void BTree::merge_leaf(Page *left_page, Page *right_page) {
    for (uint32_t i = 0; i < right_page->header->slot_cnt; i++) {
        Slot *slot = right_page->get_slot(i);
        char *record = right_page->get_record(i);

        left_page->insert_record(record, slot->size, left_page->header->slot_cnt);
    }
    left_page->header->next_page_id = right_page->header->next_page_id;
    if (right_page->header->next_page_id != 0) {
        Page *next_page = table->get_page(right_page->header->next_page_id);
        next_page->header->prev_page_id = left_page->header->page_id;
    }

    table->free_page(right_page->header->page_id);

    Page *parent = table->get_page(left_page->header->parent_id);
    remove_internal_cell(parent, right_page->header->page_id);
}

// if from_beg = true then we borrow from beginning of the src to the end of dst

void BTree::borrow_leaf(Page *src_page, Page *dst_page, bool from_beg) {
    while (dst_page->is_underflowed()) {
        uint16_t slot_index = from_beg ? 0 : src_page->header->slot_cnt - 1;
        Slot *slot = src_page->get_slot(slot_index);
        char *record = src_page->get_record(slot_index);

        dst_page->insert_record(record, slot->size, from_beg ? dst_page->header->slot_cnt : 0);
        src_page->remove_record(slot_index);
    }

    Page *right_page = from_beg ? src_page : dst_page;
    Page *left_page = from_beg ? dst_page : src_page;

    char *first_record = right_page->get_record(0);
    uint32_t new_boundary_key;
    memcpy(&new_boundary_key, first_record, sizeof(uint32_t));

    Page *parent = table->get_page(right_page->header->parent_id);
    for (uint32_t i = 0; i < parent->header->slot_cnt; i++) {
        InternalNodeCell *cell = get_internal_cell(parent, i);
        if (cell->child_page == left_page->header->page_id) {
            cell->key = new_boundary_key;
            break;
        }
    }
}

SearchResult BTree::find(uint32_t key) {
    uint32_t page_id = find_leaf_page(key);
    Page *page = table->get_page(page_id);
    int l = 0, r = page->header->slot_cnt - 1, m;
    while (l <= r) {
        m = l + (r - l) / 2;
        char *record_data = page->get_record(m);
        uint32_t record_key;
        memcpy(&record_key, record_data, sizeof(uint32_t));
        if (record_key == key) return {Cursor(table, page_id, m), true};
        if (record_key < key) l = m + 1;
        else r = m - 1;
    }
    return {Cursor(table, page_id, l), false};
}

void BTree::insert(uint32_t key, const char *record, uint16_t size) {
    SearchResult search_result = find(key);
    Cursor cursor = search_result.cursor;

    if (search_result.is_found) return;
    Page *page = table->get_page(cursor.page_num);

    if (!page->insert_record(record, size, cursor.slot_index)) {
        split_leaf_node(&cursor);
        cursor = find(key).cursor;
        page = table->get_page(cursor.page_num);
        page->insert_record(record, size, cursor.slot_index);
    }
    table->dirty_bits.set(cursor.page_num);
}

bool BTree::remove(uint32_t key) {
    SearchResult search_result = find(key);
    if (!search_result.is_found) return false;

    Page *page = table->get_page(search_result.cursor.page_num);
    page->remove_record(search_result.cursor.slot_index);

    if (page->header->parent_id != 0 && page->is_underflowed()) balance_leaf(page);
    table->dirty_bits.set(search_result.cursor.page_num);
    return true;
}

bool BTree::update(uint32_t key, const char *record, uint16_t size) {
    SearchResult search_result = find(key);
    if (!search_result.is_found) return false;

    Cursor cursor = search_result.cursor;
    Page *page = table->get_page(cursor.page_num);

    uint8_t update_result = page->update_record(record, size, cursor.slot_index);
    if (update_result == 1) {
        split_leaf_node(&cursor);
        cursor = find(key).cursor;
        page = table->get_page(cursor.page_num);
        page->update_record(record, size, cursor.slot_index);
    }
    table->dirty_bits.set(cursor.page_num);
    return true;
}

void BTree::print_tree_rec(Page *page, std::vector<std::vector<Page *> > &pages, int level) {
    if (level >= pages.size()) {
        pages.push_back(std::vector<Page *>());
    }

    pages[level].push_back(page);

    if (page->header->type != LEAF_NODE) {
        for (uint16_t i = 0; i < page->header->slot_cnt; i++) {
            InternalNodeCell *cell = get_internal_cell(page, i);
            Page *child_page = table->get_page(cell->child_page);
            print_tree_rec(child_page, pages, level + 1);
        }
        Page *child_page = table->get_page(page->header->rightmost_child_id);
        print_tree_rec(child_page, pages, level + 1);
    }
}

void BTree::print_tree() {
    std::vector<std::vector<Page *> > pages;

    uint32_t root_id = table->get_superblock()->root_page_id;
    Page *root_page = table->get_page(root_id);

    print_tree_rec(root_page, pages, 0);

    std::cout << "--------------------\n";
    std::cout << "-----  B+TREE  -----\n";
    std::cout << "--------------------\n\n";

    for (int i = 0; i < pages.size(); i++) {
        std::cout << "Level " << i << ":\n  ";

        for (int j = 0; j < pages[i].size(); j++) {
            Page *p = pages[i][j];

            std::cout << "[Pg " << pages[i][j]->header->page_id << " (";

            if (p->header->type == LEAF_NODE) {
                std::cout << "LEAF) | Keys: ";
                for (uint16_t k = 0; k < p->header->slot_cnt; k++) {
                    char *raw_record = p->get_record(k);
                    uint32_t key;
                    memcpy(&key, raw_record, sizeof(uint32_t));

                    std::cout << key;
                    if (k < p->header->slot_cnt - 1) std::cout << ",";
                }
            } else {
                std::cout << "INTERNAL) | Keys: ";
                for (uint16_t k = 0; k < p->header->slot_cnt; k++) {
                    InternalNodeCell *cell = get_internal_cell(p, k);
                    std::cout << cell->key;
                    if (k < p->header->slot_cnt) std::cout << ",";
                }
            }
            std::cout << " | Empty space: " << p->header->garbage_cnt + p->header->upper_offset - p->header->
                        lower_offset << ' ';
            std::cout << "]   ";
        }
        std::cout << "\n\n";
    }
}


SearchResult BTree::begin() {
    uint32_t current_page_id = table->get_superblock()->root_page_id;
    Page *page = table->get_page(current_page_id);

    while (page->header->type != LEAF_NODE) {
        InternalNodeCell *first_cell = get_internal_cell(page, 0);
        current_page_id = first_cell->child_page;
        page = table->get_page(current_page_id);
    }

    Cursor cursor = Cursor(table, current_page_id, 0);
    if (page->header->slot_cnt > 0) return {cursor, true};
    return {cursor, false};
}
