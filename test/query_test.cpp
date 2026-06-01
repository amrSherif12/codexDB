#include <string>
#include <iostream>
#include <sstream>

#include "../include/btree.h"
#include "../include/statement.h"
#include "../include/table.h"
#include "test.h"

bool query_test() {
    std::remove("test_database.db");
    BufferPoolManager bpm("test_database.db");
    Table table("test_database.db", &bpm);
    BTree btree(&table);

    bool is_success = true;

    try {
        if (btree.begin().is_found) {
            std::cout << "[TEST FAILED] Found a record in an empty table\n";
            is_success = false;
        }
    } catch (...) {
        std::cout << "[TEST FAILED] Error while looking for the first record in an empty table\n";
        is_success = false;
    }

    for (uint32_t i = 0; i < 2000; i++) {
        try {
            Row row = Row(i, std::to_string(i), std::to_string(i) + "@email");
            char dest[row.get_size()];
            row.serialize(dest);
            btree.insert(i, dest, row.get_size());
        } catch (...) {
            std::cout << "[TEST FAILED] Error while inserting key: " << i << "\n";
            is_success = false;
        }
    }

    try {
        if (!btree.begin().is_found) {
            std::cout << "[TEST FAILED] Couldn't find the first record of a full table\n";
            is_success = false;
        }
    } catch (...) {
        std::cout << "[TEST FAILED] Error while looking for the first record in a nonempty table\n";
        is_success = false;
    }


    for (uint32_t i = 0; i < 2000; i++) {
        try {
            if (!btree.find(i).is_found) {
                std::cout << "[TEST FAILED] Couldn't find a record after insertion stage: " << i << "\n";
                is_success = false;
            }
        } catch (...) {
            std::cout << "[TEST FAILED] Error while searching for record after insertion stage: " << i << "\n";
            is_success = false;
        }
    }

    for (uint32_t i = 0; i < 2000; i++) {
        if (i >= 500 && i < 1500) continue;
        //if (i == 1500) btree.print_tree();
        try {
            if (!btree.remove(i)) {
                std::cout << "[TEST FAILED] Couldn't delete a record: " << i << "\n";
                is_success = false;
            }
        } catch (...) {
            std::cout << "[TEST FAILED] Error while deleting record: " << i << "\n";
            is_success = false;
        }
        try {
            if (btree.find(i).is_found) {
                std::cout << "[TEST FAILED] found a deleted a record: " << i << "\n";
                is_success = false;
            }
        } catch (...) {
            std::cout << "[TEST FAILED] Error while looking for deleted record: " << i << "\n";
            is_success = false;
        }
    }

    for (uint32_t i = 500; i < 1500; i++) {
        try {
            if (!btree.find(i).is_found) {
                std::cout << "[TEST FAILED] Deletions accidentally wiped out untouched key: " << i << "\n";
                is_success = false;
            }
        } catch (...) {
            std::cout << "[TEST FAILED] Error while looking for a record after deletion stage: " << i << "\n";
            is_success = false;
        }
    }

    for (uint32_t i = 500; i < 1500; i++) {
        Row correct_row = Row(i, std::to_string(i * 1e5), std::to_string(i * 1e5) + "@gmail");
        try {
            char dest[correct_row.get_size()];
            correct_row.serialize(dest);
            btree.update(i, dest, correct_row.get_size());
        } catch (...) {
            std::cout << "[TEST FAILED] Error while updating record: " << i << "\n";
            is_success = false;
        }

        try {
            if (!btree.find(i).is_found) {
                std::cout << "[TEST FAILED] Couldn't find a record after update stage: " << i << "\n";
                is_success = false;
            }
            Row stored_row;
            stored_row.deserialize(btree.find(i).cursor.value());
            if (stored_row != correct_row) {
                std::cout << "[TEST FAILED] Couldn't find a record after update stage: " << i << "\n";
                is_success = false;
            }
        } catch (...) {
            std::cout << "[TEST FAILED] Error while checking if updated was correct record: " << i << "\n";
            is_success = false;
        }
    }


    return is_success;
}
