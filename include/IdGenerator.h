#pragma once

#include "IIdGenerator.h"

class IdGenerator : public IIdGenerator {
  public:
    uint64_t generate();
};
