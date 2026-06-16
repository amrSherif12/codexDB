
#ifndef WHERE_EXECUTOR_H
#define WHERE_EXECUTOR_H
#include "executor.h"

class WhereExecutor : public Executor {
    std::unique_ptr<Executor> child;
    WhereClause where;
public:

    WhereExecutor(std::unique_ptr<Executor> child, WhereClause &where);
    bool init() override;
    std::optional<Tuple> next() override;
};

#endif //WHERE_EXECUTOR_H
