#include "../include/compiler.h"
#include <sstream>
#include <string.h>

PrepareResult prepare_statement(const std::string &input_buffer, Statement *statement) {
    std::istringstream iss(input_buffer);
    std::string keyword;

    iss >> keyword;
    for (char &c: keyword) c = toupper(c);

    if (keyword == "INSERT") {
        statement->type = STATEMENT_INSERT;

        iss >> statement->row.username
                >> statement->row.email;


        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
        return PREPARE_SUCCESS;
    }

    if (keyword == "SELECT") {
        std::string target;
        iss >> target;
        if (target == "*") statement->type = STATEMENT_SELECT_ALL;
        else {
            statement->type = STATEMENT_SELECT;
            try {
                statement->target_id = stoul(target);
            } catch (...) {
                return PREPARE_SYNTAX_ERROR;
            }
        }

        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
        return PREPARE_SUCCESS;
    }

    if (keyword == "UPDATE") {
        statement->type = STATEMENT_UPDATE;

        iss >> statement->row.id
                >> statement->row.username
                >> statement->row.email;

        statement->target_id = statement->row.id;

        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
        return PREPARE_SUCCESS;
    }

    if (keyword == "DELETE") {
        statement->type = STATEMENT_DELETE;

        iss >> statement->target_id;

        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}
