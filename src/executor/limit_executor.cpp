#include <utility>

#include "../../include/executor/limit_executor.h"

LimitExecutor::LimitExecutor(std::unique_ptr<Executor> child, LimitClause &limit) : child(std::move(child)),
    limit(limit), cnt(0) {
}

bool LimitExecutor::init() {
    return child->init();
}

std::optional<Tuple> LimitExecutor::next() {
    if (++cnt > limit.cnt) return std::nullopt;

    std::optional<Tuple> tuple = child->next();
    if (!tuple.has_value()) return std::nullopt;
    return tuple;
}

