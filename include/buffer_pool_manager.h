#ifndef BUFFER_POOL_MANAGER_H
#define BUFFER_POOL_MANAGER_H
#include <unordered_map>

#include "page.h"
#include "pager.h"

constexpr int POOL_SIZE = 50;

struct Frame {
    Page *page;
    uint32_t page_id;
    bool is_dirty;
    uint32_t pin_cnt;
    uint64_t last_used;
};

class BufferPoolManager {
    Frame *frames;
    size_t pool_size;
    Pager pager;
    std::unordered_map<uint32_t, size_t> page_table;
    uint64_t global_timestamp;

    size_t find_victim_frame();

public:
    BufferPoolManager(const std::string &file_name);

    ~BufferPoolManager();

    Page *fetch_page(uint32_t page_id);

    Page *allocate_page(uint32_t page_id);

    void unpin_page(uint32_t page_id, bool is_dirty);

    bool flush_page(uint32_t page_id);

    void flush_all_pages();

    void init_file();

    bool is_new_file();

    void print_frames();
};

#endif //BUFFER_POOL_MANAGER_H
