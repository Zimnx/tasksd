#pragma once

#include <memory>
#include <stdint.h>
#include <folly/io/IOBuf.h>
#include "IClientHandler.h"

class IDataStorage {
  public:
    using TaskId = uint64_t;
    using Task = std::shared_ptr<folly::IOBuf>;

    struct ClientTask {
        ClientTask(TaskId taskId_, Task task_)
          : taskId(taskId_)
          , task(std::move(task_))
        {}

        TaskId taskId;
        Task task;
    };

    virtual ~IDataStorage() = default;

    virtual void markDone(IClientHandler::ClientId clientId, TaskId taskId) = 0;
    virtual void put(Task task) = 0;
    virtual ClientTask get(IClientHandler::ClientId clientId) const = 0;
};
