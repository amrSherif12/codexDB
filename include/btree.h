#ifndef BTREE_H
#define BTREE_H
#include <vector>

#include "cursor.h"

struct InternalNodeCell {
    uint32_t child_page;
    uint32_t key;
};

struct SplitResult {
    uint32_t new_page_id;
    uint32_t pushed_up_key;
};

struct SearchResult {
    Cursor cursor;
    bool is_found;
};

class BTree {
    InternalNodeCell *get_internal_cell(Page *node, uint16_t cell_num);

    bool insert_internal_cell(Page *node, uint32_t key, uint32_t left_child, uint32_t right_child);

    void remove_internal_cell(Page *parent, uint32_t child_to_delete);


    uint32_t find_leaf_page(uint32_t key, uint32_t page_id = 0);

    uint16_t get_child_index(Page *parent, uint16_t child_page_id);

    void balance_leaf(Page *leaf_page);

    SplitResult split_leaf_node(Cursor *cursor);

    void merge_leaf(Page *left_page, Page *right_page);

    void borrow_leaf(Page *src_page, Page *dst_page, bool from_beg);

    void balance_internal(Page *internal_page);

    SplitResult split_internal_node(uint32_t internal_page_id);

    void merge_internal(Page *left, Page *right, Page *parent, uint16_t parent_index);

    void borrow_internal(Page *src, Page *dst, Page *parent, uint16_t parent_index, bool from_beg);

    void print_tree_rec(Page *page, std::vector<std::vector<Page*>> &pages, int level);

public:
    Table *table;

    BTree(Table *table);

    SearchResult find(uint32_t key);

    void insert(uint32_t key, const char *record, uint16_t size);

    bool remove(uint32_t key);

    bool update(uint32_t key, const char *record, uint16_t size);

    void print_tree();

    SearchResult begin();
};

#endif //BTREE_H
