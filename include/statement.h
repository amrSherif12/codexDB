#ifndef STATEMENT_H
#define STATEMENT_H
#include <any>
#include <cstdint>
#include <memory>
#include <vector>

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
    STATEMENT_UPDATE_ALL,
    STATEMENT_DELETE,
    STATEMENT_DELETE_ALL,
};

enum ComparisonType {
    COMPARISON_EQUAL,
    COMPARISON_NOT_EQUAL,
    COMPARISON_GREATER,
    COMPARISON_GREATER_OR_EQUAL,
    COMPARISON_SMALLER,
    COMPARISON_SMALLER_OR_EQUAL,
};

enum ClauseType {
    CLAUSE_WHERE,
    CLAUSE_LIMIT,
};

struct Clause {
    ClauseType type;
    virtual ~Clause() = default;
};

struct WhereClause : Clause {
    std::string column;
    ComparisonType comparison;
    uint32_t value;
};

struct LimitClause : Clause {
    uint32_t cnt;
};

struct OrderByClause : Clause {
    std::string column;
};

struct Row {
    uint32_t id;
    std::string username;
    std::string email;
    std::vector<std::any> v;


    uint16_t get_size() const {
        return sizeof(uint32_t) +
               sizeof(uint16_t) + username.length() +
               sizeof(uint16_t) + email.length();
    }

    bool operator==(Row &other) {
        return
                this->id == other.id &&
                this->email == other.email &&
                this->username == other.username;
    }

    void serialize(char *dest)  {
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

    void deserialize(const char *source) {
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
    StatementType statement;
    std::vector<std::unique_ptr<Clause>> clauses;
    Row row;
    uint32_t target_id;
};

#endif //STATEMENT_H
