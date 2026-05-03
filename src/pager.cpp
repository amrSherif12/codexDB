#include "../include/pager.h"

Pager::Pager(const std::string &file_name) {
    std::ifstream check(file_name);
    if (!check.is_open()) std::ofstream create(file_name);
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekg(0, std::ios::end);
    file_length = file.tellg();
}

Pager::~Pager() {
    if (file.is_open()) file.close();
}

void Pager::read_page(int page_num, char *destination) {
    file.seekg(page_num * PAGE_SIZE, std::ios::beg);
    file.read(destination, PAGE_SIZE);
}

void Pager::write_page(int page_num, char *source) {
    file.seekp(page_num * PAGE_SIZE, std::ios::beg);
    file.write(source, PAGE_SIZE);
    file.flush();
    int new_size = (page_num + 1) * PAGE_SIZE;
    if (new_size > file_length) file_length = new_size;
}
