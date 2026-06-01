#ifndef PAGE_H
#define PAGE_H
#include <cstdint>

#pragma pack(push, 1)
enum PageType : uint8_t {
    FREE_NODE,
    INTERNAL_NODE,
    LEAF_NODE,
    SUPER_BLOCK,
};

struct PageHeader {
    PageType type;
    uint16_t slot_cnt;
    uint16_t lower_offset; // free space start
    uint16_t upper_offset; // free space end
    uint16_t garbage_cnt;
    uint32_t page_id;
    uint32_t parent_id;
    uint32_t next_page_id;
    uint32_t prev_page_id;
    uint32_t rightmost_child_id;
};

struct Slot {
    uint16_t offset;
    uint16_t size;
};

struct SuperBlock {
    uint32_t root_page_id;
    uint32_t first_free_page;
    uint32_t next_auto_increment_row_id;
    uint32_t next_auto_increment_page_id;
};
#pragma pack(pop)

class Page {
public:
    char *data;
    PageHeader *header;

    Page(char *data);

    ~Page();

    Slot *get_slot(uint16_t slot_index);

    char *get_record(uint16_t slot_index);

    bool insert_record(const char *record_data, uint16_t size, uint16_t slot_index);

    void remove_record(uint16_t slot_index);

    uint8_t update_record(const char *record_data, uint16_t size, uint16_t slot_index);

    bool is_underflowed();

    void defragment();

    uint32_t leaf_page_size();

    void init_new_page(PageType type, uint32_t page_id);
};

#endif //PAGE_H
