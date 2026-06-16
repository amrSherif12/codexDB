
#ifndef LIMIT_EXECUTOR_H
#define LIMIT_EXECUTOR_H
#include "executor.h"

class LimitExecutor : public Executor {
    std::unique_ptr<Executor> child;
    LimitClause limit;
    uint32_t cnt;
public:

    LimitExecutor(std::unique_ptr<Executor> child, LimitClause &limit);
    bool init() override;
    std::optional<Tuple> next() override;
};

#endif //LIMIT_EXECUTOR_H
