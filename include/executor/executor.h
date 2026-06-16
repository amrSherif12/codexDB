#ifndef EXECUTOR_H
#define EXECUTOR_H
#include <optional>

#include "../statement.h"

struct Tuple {
    Row row;
};

class Executor {
public:
    virtual ~Executor() = default;
    virtual bool init() = 0;
    virtual std::optional<Tuple> next() = 0;
};

#endif //EXECUTOR_H
