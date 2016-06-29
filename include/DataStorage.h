#pragma once

#include <deque>
#include <unordered_map>
#include <shared_mutex>
#include <thread>

#include "IService.h"
#include "IClientHandler.h"
#include "IDataStorage.h"

class IIdGenerator;

class DataStorage : public IDataStorage, public IService, public IClientHandler {
  public:
    DataStorage(std::unique_ptr<IIdGenerator> idGenerator);

    void markDone(ClientId clientId, TaskId taskId);
    void put(Task task);
    ClientTask get(ClientId clientId) const;

    void registerClient(ClientId clientId);
    void unregisterClient(ClientId clientId);

    void start();
    void shutDown();

  private:

    using ClientTaskQueue = std::deque<ClientTask>;

    std::unique_ptr<IIdGenerator> m_idGenerator;
    std::deque<ClientTask> m_tasks;
    std::unordered_map<ClientId, ClientTaskQueue>  m_clientTasks;
    mutable std::shared_timed_mutex m_mutex;

    std::thread m_taskDispatcherThread;
    std::atomic<bool> m_dispatcherEnabled;

    void taskDispatcherWork();
};

