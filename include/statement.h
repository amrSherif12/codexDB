#ifndef STATEMENT_H
#define STATEMENT_H
#include <cstdint>

enum PrepareResult {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT,
};

enum ExecuteResult {
    EXECUTE_SUCCESS,
    EXECUTE_NOT_FOUND,
};

enum StatementType {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATEMENT_SELECT_ALL,
    STATEMENT_UPDATE,
    STATEMENT_DELETE,
};

struct Row {
    uint32_t id;
    std::string username;
    std::string email;

    uint16_t get_size() const {
        return sizeof(uint32_t) +
               sizeof(uint16_t) + username.length() +
               sizeof(uint16_t) + email.length();
    }

    void serialize(char* dest) const {
        uint16_t offset = 0;

        memcpy(dest + offset, &id, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        uint16_t un_len = username.length();
        memcpy(dest + offset, &un_len, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        memcpy(dest + offset, username.c_str(), un_len);
        offset += un_len;

        uint16_t em_len = email.length();
        memcpy(dest + offset, &em_len, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        memcpy(dest + offset, email.c_str(), em_len);
    }

    void deserialize(const char* source) {
        uint16_t offset = 0;

        memcpy(&id, source + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        uint16_t un_len;
        memcpy(&un_len, source + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        username = std::string(source + offset, un_len);
        offset += un_len;

        uint16_t em_len;
        memcpy(&em_len, source + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        email = std::string(source + offset, em_len);
    }
};

struct Statement {
    StatementType type;
    Row row;
    uint32_t target_id;
};

#endif //STATEMENT_H
