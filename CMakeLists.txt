cmake_minimum_required(VERSION 3.30)
project(cool_dbms)

set(CMAKE_CXX_STANDARD 20)

add_executable(cool_dbms src/main.cpp
        src/pager.cpp
        src/cursor.cpp
        src/table.cpp
        include/pager.h
        include/table.h
        include/cursor.h
        include/page.h
        src/page.cpp
        include/btree.h
        src/btree.cpp
        include/statement.h
        include/compiler.h
        src/compiler.cpp
        include/buffer_pool_manager.h
        src/buffer_pool_manager.cpp
        include/page_guard.h
        src/page_guard.cpp
        test/query_test.cpp
        test/test.cpp
        test/test.h
        include/executor/executor.h
        include/executor/scan_executor.h
        src/executor/scan_executor.cpp
        include/executor/where_executor.h
        src/executor/where_executor.cpp
        include/executor/limit_executor.h
        src/executor/limit_executor.cpp
        include/executor/execution_engine.h
        src/executor/execution_engine.cpp
        include/executor/update_executor.h
        "include/executor/insert_executor.h"
        include/executor/delete_executor.h
        src/executor/insert_executor.cpp
        src/executor/update_executor.cpp
        src/executor/delete_executor.cpp
)
