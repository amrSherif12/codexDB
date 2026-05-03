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
        include/executor.h
        src/compiler.cpp
        src/executor.cpp)
