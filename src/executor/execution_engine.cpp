#include "../../include/executor/execution_engine.h"

#include <iostream>

#include "../../include/executor/delete_executor.h"
#include "../../include/executor/insert_executor.h"
#include "../../include/executor/limit_executor.h"
#include "../../include/executor/scan_executor.h"
#include "../../include/executor/update_executor.h"
#include "../../include/executor/where_executor.h"

std::unique_ptr<Executor> ExecutionEngine::build_pipeline(Statement &statement, BTree *btree) {
    std::unique_ptr<Executor> root;

    if (statement.statement == STATEMENT_INSERT) {
        Tuple tuple = {statement.row};
        root = std::make_unique<InsertExecutor>(tuple, btree);
        return root;
    }
    root = std::make_unique<ScanExecutor>(btree);
    auto *scan_executor = dynamic_cast<ScanExecutor *>(root.get());

    for (std::unique_ptr<Clause> &clause: statement.clauses) {
        if (clause->type == CLAUSE_WHERE) {
            auto *where = dynamic_cast<WhereClause *>(clause.get());
            root = std::make_unique<WhereExecutor>(std::move(root), *where);
        } else if (clause->type == CLAUSE_LIMIT) {
            auto *limit = dynamic_cast<LimitClause *>(clause.get());
            root = std::make_unique<LimitExecutor>(std::move(root), *limit);
        }
    }

    if (statement.statement == STATEMENT_DELETE_ALL) root = std::make_unique<DeleteExecutor>(std::move(root), scan_executor, btree);
    if (statement.statement == STATEMENT_UPDATE_ALL) {
        Tuple new_tuple{statement.row};
        root = std::make_unique<UpdateExecutor>(std::move(root), new_tuple , btree);
    };

    return root;
}

void ExecutionEngine::execute(Statement &statement, BTree *btree) {
    std::unique_ptr<Executor> root = build_pipeline(statement, btree);

    if (!root->init()) {
        std::cout << "Database Empty!\n";
        return;
    }
    uint32_t rows_cnt = 0;

    while (true) {
        std::optional<Tuple> tuple = root->next();
        if (!tuple.has_value()) break;

        rows_cnt++;
        std::cout << tuple->row.id << " " << tuple->row.username << " " << tuple->row.email << '\n';
    }
    if (rows_cnt == 0)std::cout << "No rows affected" << '\n';
    else std::cout << rows_cnt << '\n';
}
