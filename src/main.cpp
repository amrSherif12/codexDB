#include <string>
#include <iostream>

#include "../include/btree.h"
#include "../include/compiler.h"
#include "../include/executor.h"
#include "../include/statement.h"
#include "../include/table.h"

#ifdef _WIN32
#include <windows.h>
#endif


void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif
}

int main() {
    setupConsole();
    std::string input_buffer;

std::string yellow =  "\033[38;2;255;180;0m";
    std::string red = "\033[1;31m";
    std::string reset = "\033[0m";
    std::string bold = "\033[1m";

    std::cout << "\n";
    std::cout << yellow << "   ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗" << reset << "\n";
    std::cout << yellow << "  ██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝" << reset << "\n";
    std::cout << yellow << "  ██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ " << reset << "\n";
    std::cout << yellow << "  ██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ " << reset << "\n";
    std::cout << yellow << "  ╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗" << reset << "\n";
    std::cout << yellow << "   ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝" << reset << "\n";
    std::cout << "\n";

    std::cout <<
            "codexDB is a simple DBMS\n- It supports only one built in table (int id, string username, string email)\n- The basic CRUD operations\n- Type .help to see all functionality\n\n";

    Table *table = new Table("database.db");
    BTree btree(table);

    while (true) {
        std::cout << yellow << "codexDB> " << reset;
        std::getline(std::cin, input_buffer);

        if (input_buffer.empty()) continue;
        if (input_buffer[0] == '.') {
            if (input_buffer == ".exit") {
                delete table;
                std::cout << "Exiting...\n";
                break;
            }
            if (input_buffer == ".help") {
                std::cout << "---------------\n";
                std::cout << "|  HELP MENU  |\n";
                std::cout << "---------------\n\n";
                std::cout << "-- META COMMANDS --\n";
                std::cout << "- .help: opens the help menu\n";
                std::cout << "- .save: saves the changes you made to the db to disc if you dont save changes will stay in memory and never get saved\n";
                std::cout << "- .exit: does the same thing as .save but ends the program too\n";
                std::cout << "- .tree: prints the structure of the B+tree in the console (I made this for debugging but its cool to see if you want)\n\n";
                std::cout << "-- COMMANDS --\n";
                std::cout << "SELECT * (gets all records)/ <id>\n";
                std::cout << "INSERT <name> <email>\n";
                std::cout << "UPDATE <id> <name> <email>\n";
                std::cout << "DELETE <id>\n";
                continue;
            } else if (input_buffer == ".tree") {
                btree.print_tree();
                continue;
            } else {
                std::cout << red << "Unrecognized command " << reset << input_buffer << "'.\n";
            }
        }

        Statement statement;

        PrepareResult prepare_result = prepare_statement(input_buffer, &statement);

        if (prepare_result == PREPARE_SYNTAX_ERROR) {
            std::cout << red << "Syntax error.\n" << reset;
            continue;
        }
        if (prepare_result == PREPARE_UNRECOGNIZED_STATEMENT) {
            std::cout << red << "Unrecognised command.\n" << reset;
            continue;
        }

        ExecuteResult execute_result = execute_statement(&statement, &btree);

        if (execute_result == EXECUTE_SUCCESS) {
            std::cout << "Executed.\n";
        } else if (execute_result == EXECUTE_NOT_FOUND) {
            std::cout << red << "Error: Record not found.\n" << reset;
        }
    }
}
