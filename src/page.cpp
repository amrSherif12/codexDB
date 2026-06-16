#include "../include/page.h"
#include "../include/btree.h"
#include "../include/pager.h"

Page::Page(char *data) {
    this->data = data;
    header = reinterpret_cast<PageHeader *>(data);
}

Page::~Page() {
    delete[] data;
}

Slot *Page::get_slot(uint16_t slot_index) {
    char *slot_data = data + sizeof(PageHeader) + slot_index * sizeof(Slot);
    return reinterpret_cast<Slot *>(slot_data);
}

char *Page::get_record(uint16_t slot_index) {
    Slot *slot = get_slot(slot_index);
    if (!slot) return nullptr;
    return data + slot->offset;
}


bool Page::insert_record(const char *record_data, uint16_t size, uint16_t slot_index) {
    uint16_t space_needed = size + sizeof(Slot);
    if (header->upper_offset - header->lower_offset < space_needed) {
        if ((header->upper_offset - header->lower_offset) + header->garbage_cnt < size + sizeof(Slot)) return false;
        this->defragment();
    }
    header->upper_offset -= size;
    memcpy(data + header->upper_offset, record_data, size);

    char *src = data + sizeof(PageHeader) + sizeof(Slot) * slot_index;
    char *dst = data + sizeof(PageHeader) + sizeof(Slot) * (slot_index + 1);
    memmove(dst, src, (header->slot_cnt - slot_index) * sizeof(Slot));
    Slot *new_slot = reinterpret_cast<Slot *>(data + sizeof(PageHeader) + sizeof(Slot) * slot_index);
    new_slot->offset = header->upper_offset;
    new_slot->size = size;
    header->lower_offset += sizeof(Slot);
    header->slot_cnt++;
    return true;
}

void Page::remove_record(uint16_t slot_index) {
    Slot *slot = get_slot(slot_index);
    if (slot_index >= header->slot_cnt) return;
    Slot *next_slot = get_slot(slot_index + 1);
    header->garbage_cnt += slot->size;
    memmove(slot, next_slot, (header->slot_cnt - slot_index) * sizeof(Slot));
    header->slot_cnt--;
    header->lower_offset -= sizeof(Slot);
}

// returns two flags
// not enough space (first bit)
// record is deleted or doesn't exist (second bit)
// 0 for success

uint8_t Page::update_record(const char *record_data, uint16_t size, uint16_t slot_index) {
    if (slot_index >= header->slot_cnt) return 1 << 1;
    Slot *slot = get_slot(slot_index);

    if (slot->size >= size) {
        memcpy(data + slot->offset, record_data, size);
        header->garbage_cnt += slot->size - size;
        slot->size = size;
        return 0;
    }

    if (header->upper_offset - header->lower_offset < size) {
        if (header->garbage_cnt + (header->upper_offset - header->lower_offset) < size) return 1;
        this->defragment();
        slot = get_slot(slot_index);
    }
    header->garbage_cnt += slot->size;
    header->upper_offset -= size;
    memcpy(data + header->upper_offset, record_data, size);
    slot->offset = header->upper_offset;
    slot->size = size;
    return 0;
}

bool Page::is_underflowed() {
    uint32_t total_space = PAGE_SIZE - sizeof(PageHeader);
    uint32_t taken_space = 0;
    if (header->type == LEAF_NODE)
        taken_space = header->slot_cnt * sizeof(Slot) + PAGE_SIZE - header->upper_offset -
                      header->garbage_cnt;
    else taken_space = header->slot_cnt * sizeof(InternalNodeCell);

    return taken_space < total_space / 2;
}

void Page::defragment() {
    char temp[PAGE_SIZE];
    uint16_t temp_upper = PAGE_SIZE;
    for (uint16_t i = 0; i < header->slot_cnt; i++) {
        Slot *slot = get_slot(i);
        temp_upper -= slot->size;
        memcpy(temp + temp_upper, data + slot->offset, slot->size);
        slot->offset = temp_upper;
    }
    memcpy(data + temp_upper, temp + temp_upper, PAGE_SIZE - temp_upper);
    header->garbage_cnt = 0;
    header->upper_offset = temp_upper;
}

uint32_t Page::leaf_page_size() {
    uint32_t page_size = 0;
    for (int i = 0; i < header->slot_cnt; i++) {
        Slot *slot = get_slot(i);
        page_size += slot->size + sizeof(Slot);
    }
    return page_size;
}

void Page::init_new_page(PageType type, uint32_t page_id) {
    header->type = type;
    header->page_id = page_id;
    header->lower_offset = sizeof(PageHeader);
    header->upper_offset = PAGE_SIZE;
    header->slot_cnt = 0;
    header->garbage_cnt = 0;
    header->next_page_id = 0;
    header->prev_page_id = 0;
    header->parent_id = 0;
    header->rightmost_child_id = 0;
}
