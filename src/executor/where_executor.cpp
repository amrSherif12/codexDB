#include <utility>

#include "../../include/executor/where_executor.h"

WhereExecutor::WhereExecutor(std::unique_ptr<Executor> child, WhereClause &where) : child(std::move(child)),
    where(where) {
}

bool WhereExecutor::init() {
    return child->init();
}

std::optional<Tuple> WhereExecutor::next() {
    while (true) {
        std::optional<Tuple> tuple = child->next();
        if (!tuple.has_value()) {
            return std::nullopt;
        }
        bool matches = false;
        switch (where.comparison) {
            case COMPARISON_EQUAL:
                matches = tuple->row.id == where.value;
                break;
            case COMPARISON_NOT_EQUAL:
                matches = tuple->row.id != where.value;
                break;
            case COMPARISON_GREATER:
                matches = tuple->row.id > where.value;
                break;
            case COMPARISON_GREATER_OR_EQUAL:
                matches = tuple->row.id >= where.value;
                break;
            case COMPARISON_SMALLER:
                matches = tuple->row.id < where.value;
                break;
            case COMPARISON_SMALLER_OR_EQUAL:
                matches = tuple->row.id <= where.value;
                break;
        }

        if (matches) {
            return tuple;
        }
    }
}
