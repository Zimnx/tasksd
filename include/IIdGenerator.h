#pragma once

#include <stdint.h>

class IIdGenerator {
  public:
    virtual ~IIdGenerator() = default;

    virtual uint64_t generate() = 0;
};
