#include "../include/compiler.h"
#include <sstream>
#include <string.h>

PrepareResult prepare_statement(const std::string &input_buffer, Statement *statement) {
    std::istringstream iss(input_buffer);
    std::string statement_str;

    iss >> statement_str;
    for (char &c: statement_str) c = toupper(c);

    if (statement_str == "INSERT") {
        statement->statement = STATEMENT_INSERT;

        iss >> statement->row.username
                >> statement->row.email;

        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
    }

    else if (statement_str == "SELECT") {
        std::string target;
        iss >> target;
        if (target == "*") statement->statement = STATEMENT_SELECT_ALL;
        else {
            statement->statement = STATEMENT_SELECT;
            try {
                statement->target_id = stoul(target);
            } catch (...) {
                return PREPARE_SYNTAX_ERROR;
            }
        }
        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
    }
    
    else if (statement_str == "UPDATE") {
        std::string target;
        iss >> target;
        if (target == "*") {
            statement->statement = STATEMENT_UPDATE_ALL;
            iss
                >> statement->row.username
                >> statement->row.email;
        }
        else {
            statement->statement = STATEMENT_UPDATE;
            try {
                statement->target_id = stoul(target);
            } catch (...) {
                return PREPARE_SYNTAX_ERROR;
            }

        }
        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
    }

    else if (statement_str == "DELETE") {
        std::string target;
        iss >> target;
        if (target == "*") statement->statement = STATEMENT_DELETE_ALL;
        else {
            statement->statement = STATEMENT_DELETE;
            try {
                statement->target_id = stoul(target);
            } catch (...) {
                return PREPARE_SYNTAX_ERROR;
            }
            iss
                >> statement->row.username
                >> statement->row.email;
        }
        if (iss.fail()) return PREPARE_SYNTAX_ERROR;
    }

    else return PREPARE_UNRECOGNIZED_STATEMENT;


    while (true) {
        std::string clause_str;
        iss >> clause_str;
        if (iss.fail()) return PREPARE_SUCCESS;
        for (char &c: clause_str) c = toupper(c);

        if (clause_str == "WHERE") {
            WhereClause clause;
            clause.type = CLAUSE_WHERE;

            std::string comparison;
            iss >> comparison;
            if (comparison == "=") clause.comparison = COMPARISON_EQUAL;
            else if (comparison == "!=") clause.comparison = COMPARISON_NOT_EQUAL;
            else if (comparison == ">") clause.comparison = COMPARISON_GREATER;
            else if (comparison == ">=") clause.comparison = COMPARISON_GREATER_OR_EQUAL;
            else if (comparison == "<") clause.comparison = COMPARISON_SMALLER;
            else if (comparison == "<=") clause.comparison = COMPARISON_SMALLER_OR_EQUAL;
            else return PREPARE_SYNTAX_ERROR;

            iss >> clause.value;

            if (iss.fail()) return PREPARE_SYNTAX_ERROR;
            statement->clauses.push_back(std::make_unique<WhereClause>(clause));
        }

        else if (clause_str == "LIMIT") {
            LimitClause clause;
            clause.type = CLAUSE_LIMIT;

            iss >> clause.cnt;

            if (iss.fail()) return PREPARE_SYNTAX_ERROR;
            statement->clauses.push_back(std::make_unique<LimitClause>(clause));
        }
    }
}
