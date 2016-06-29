#pragma once

#include <stdint.h>

class IClientHandler {
  public:
    using ClientId = uint32_t;

    virtual ~IClientHandler() = default;

    virtual void registerClient(ClientId clientId) = 0;
    virtual void unregisterClient(ClientId clientId) = 0;
};
