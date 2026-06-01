#include "../include/buffer_pool_manager.h"
#include <iostream>
#include <iomanip>

BufferPoolManager::BufferPoolManager(const std::string &file_name) : pager(file_name) {
    frames = new Frame[POOL_SIZE];
    global_timestamp = 1;
    for (int i = 0; i < POOL_SIZE; i++) {
        char *data = new char[PAGE_SIZE];
        frames[i].page = new Page(data);
        frames[i].is_dirty = false;
        frames[i].pin_cnt = 0;
        frames[i].last_used = 0;
        frames[i].page_id = 0;
    }
}

BufferPoolManager::~BufferPoolManager() {
    flush_all_pages();
    for (int i = 0; i < POOL_SIZE; i++) {
        delete frames[i].page;
    }
    delete[] frames;
}

size_t BufferPoolManager::find_victim_frame() {
    size_t victim_idx = -1;
    uint64_t lowest_timestamp = UINT64_MAX;

    for (int i = 0; i < POOL_SIZE; i++) {
        if (frames[i].last_used == 0) return i;
        if (frames[i].pin_cnt == 0 && frames[i].last_used < lowest_timestamp) {
            lowest_timestamp = frames[i].last_used;
            victim_idx = i;
        }
    }

    if (victim_idx == -1) {
        throw std::runtime_error("Out of memory. Buffer Pool is completely full and all pages are pinned.");
    }

    flush_page(frames[victim_idx].page_id);
    page_table.erase(frames[victim_idx].page_id);

    return victim_idx;
}

Page *BufferPoolManager::fetch_page(uint32_t page_id) {
    if (page_table.contains(page_id)) {
        size_t frame_idx = page_table[page_id];
        frames[frame_idx].pin_cnt++;
        frames[frame_idx].last_used = global_timestamp++;
        return frames[frame_idx].page;
    }

    size_t victim_idx = find_victim_frame();
    Frame &frame = frames[victim_idx];

    pager.read_page(page_id, frame.page->data);

    frame.is_dirty = false;
    frame.pin_cnt = 1;
    frame.last_used = global_timestamp++;
    frame.page_id = page_id;
    page_table[page_id] = victim_idx;

    return frame.page;
}

Page *BufferPoolManager::allocate_page(uint32_t page_id) {
    if (page_table.contains(page_id)) {
        size_t frame_idx = page_table[page_id];
        frames[frame_idx].pin_cnt++;
        frames[frame_idx].last_used = global_timestamp++;
        return frames[frame_idx].page;
    }

    size_t frame_idx = find_victim_frame();
    Frame &frame = frames[frame_idx];

    frame.is_dirty = false;
    frame.last_used = global_timestamp++;
    frame.pin_cnt = 1;
    frame.page_id = page_id;
    page_table[page_id] = frame_idx;
    pager.file_length += PAGE_SIZE;

    return frame.page;
}

void BufferPoolManager::unpin_page(uint32_t page_id, bool is_dirty) {
    if (page_table.contains(page_id)) {
        size_t frame_idx = page_table[page_id];

        if (frames[frame_idx].pin_cnt > 0) frames[frame_idx].pin_cnt--;
        frames[frame_idx].is_dirty |= is_dirty;
    }
}

bool BufferPoolManager::flush_page(uint32_t page_id) {
    if (page_table.contains(page_id)) {
        size_t frame_idx = page_table[page_id];
        if (frames[frame_idx].pin_cnt > 0) throw std::runtime_error("Trying to flush a page with pin_cnt > 0.");
        if (frames[frame_idx].is_dirty) {
            pager.write_page(page_id, frames[frame_idx].page->data);
            frames[frame_idx].is_dirty = false;
        }
        return true;
    }
    return false;
}

void BufferPoolManager::flush_all_pages() {
    for (auto [page_id, idx] : page_table) {
        flush_page(page_id);
    }
}

void BufferPoolManager::init_file() {
    char sp_buff[PAGE_SIZE] = {0};
    SuperBlock *superblock = reinterpret_cast<SuperBlock *>(sp_buff);

    superblock->root_page_id = 1;
    superblock->next_auto_increment_row_id = 0;
    superblock->next_auto_increment_page_id = 2;
    superblock->first_free_page = 0;

    pager.write_page(0, sp_buff);

    char *pg_buff = new char[PAGE_SIZE];
    Page page(pg_buff);
    page.init_new_page(LEAF_NODE, 1);
    pager.write_page(1, pg_buff);

    pager.file_length += PAGE_SIZE * 2;
}

bool BufferPoolManager::is_new_file() {
    return pager.file_length == 0;
}

void BufferPoolManager::print_frames() {
    std::cout << "\n===============================================================\n";
    std::cout << "                      BUFFER POOL STATE                        \n";
    std::cout << "===============================================================\n";
    std::cout << "| Frame | Page ID | Dirty | Pin Cnt |      Last Used          |\n";
    std::cout << "|-------|---------|-------|---------|-------------------------|\n";

    for (int i = 0; i < POOL_SIZE; i++) {
        const Frame &frame = frames[i];
        std::cout << "| " << std::setw(5) << i << " | ";
        if (frame.last_used == 0 && frame.pin_cnt == 0) {
            std::cout << std::setw(7) << "-" << " | "
                      << std::setw(5) << "-" << " | "
                      << std::setw(7) << "-" << " | "
                      << std::setw(23) << "[ EMPTY ]" << " |\n";
        } else {
            std::cout << std::setw(7) << frame.page_id << " | "
                      << std::setw(5) << (frame.is_dirty ? "YES" : "NO") << " | "
                      << std::setw(7) << frame.pin_cnt << " | "
                      << std::setw(23) << frame.last_used << " |\n";
        }
    }
    std::cout << "===============================================================\n\n";
}

