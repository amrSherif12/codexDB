#ifndef PAGER_H
#define PAGER_H

#include<fstream>

constexpr int PAGE_SIZE = 8192;

class Pager {
    std::fstream file;

public:
    int file_length;

    Pager(const std::string &file_name);

    ~Pager();

    void read_page(int page_num, char *destination);

    void write_page(int page_num, char *source);
};

#endif //PAGER_H
