#include <iostream>

#include "../include/executor.h"
#include "../include/compiler.h"

ExecuteResult execute_statement(Statement *statement, BTree *btree) {
    switch (statement->type) {
        case STATEMENT_INSERT: {
            uint32_t id = btree->table->get_superblock()->next_auto_increment_id;
            btree->table->increment_superblock_id();
            statement->row.id = id;

            uint16_t size = statement->row.get_size();
            char *buffer = new char[size];
            statement->row.serialize(buffer);

            btree->insert(statement->row.id, buffer, size);

            delete[] buffer;
            return EXECUTE_SUCCESS;
        }
        case STATEMENT_SELECT: {
            SearchResult search_result = btree->find(statement->target_id);
            Cursor cursor = search_result.cursor;

            if (!search_result.is_found) {
                return EXECUTE_NOT_FOUND;
            }
            Row row;
            row.deserialize(cursor.value());

            std::cout << "(" << row.id << ", " << row.username << ", " << row.email << ")\n";
            return EXECUTE_SUCCESS;
        }
        case STATEMENT_SELECT_ALL: {
            SearchResult search_result = btree->begin();
            Cursor cursor = search_result.cursor;
            int count = 0;

            if (!search_result.is_found) {
                return EXECUTE_NOT_FOUND;
            }

            while (!cursor.end_of_table()) {
                Row row;
                row.deserialize(cursor.value());

                std::cout << "(" << row.id << ", " << row.username << ", " << row.email << ")\n";
                count++;

                cursor.advance();
            }
            std::cout << "Fetched " << count << " rows.\n";
            return EXECUTE_SUCCESS;
        }
        case STATEMENT_UPDATE: {
            uint16_t size = statement->row.get_size();
            char *buffer = new char[size];
            statement->row.serialize(buffer);

            if (!btree->update(statement->target_id, buffer, size)) return EXECUTE_NOT_FOUND;

            delete[] buffer;
            return EXECUTE_SUCCESS;
        }
        case STATEMENT_DELETE: {
            if (!btree->remove(statement->target_id)) return EXECUTE_NOT_FOUND;
            return EXECUTE_SUCCESS;
        }
    }
    return EXECUTE_SUCCESS;
}
