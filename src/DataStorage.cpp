#include "DataStorage.h"
#include "IIdGenerator.h"

DataStorage::DataStorage(std::unique_ptr<IIdGenerator> idGenerator)
  : m_idGenerator(std::move(idGenerator))
  , m_tasks{}
  , m_clientTasks{}
  , m_mutex {}
  , m_dispatcherEnabled { false }
{}

void DataStorage::put(Task task) {
  std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
  TaskId taskId = m_idGenerator->generate();
  m_tasks.emplace_back(taskId, task);
}

IDataStorage::ClientTask DataStorage::get(ClientId clientId) const {
  std::shared_lock<std::shared_timed_mutex> sharedLock(m_mutex);

  auto clientTasksIt = m_clientTasks.find(clientId);
  if (clientTasksIt == m_clientTasks.end()) {
    return {0, nullptr}; // TODO: return optional
  }

  const ClientTaskQueue& clientTasks = clientTasksIt->second;
  const ClientTask& clientTask = clientTasks.front();

  return {clientTask.taskId, clientTask.task};
  // TODO: return multiple tasks, for better throughput
}

void DataStorage::registerClient(ClientId clientId) {
  std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
  m_clientTasks[clientId] = {};
}

void DataStorage::unregisterClient(ClientId clientId) {
  std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
  auto clientTasksIt = m_clientTasks.find(clientId);
  if (clientTasksIt == m_clientTasks.end()) {
    return;
  }

  m_clientTasks.erase(clientTasksIt);
}

void DataStorage::start() {
  m_dispatcherEnabled.store(true);
  m_taskDispatcherThread = std::thread{&DataStorage::taskDispatcherWork, this};
}

void DataStorage::shutDown() {
  m_dispatcherEnabled.store(false);
  m_taskDispatcherThread.join();
  std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
  for (auto& clientPair : m_clientTasks) {
    ClientTaskQueue& clientQueue = clientPair.second;
    while (!clientQueue.empty()) {
      m_tasks.push_back(clientQueue.front());
      clientQueue.pop_front();
    }
  }
  // move left over tasks to errorLog
}

void DataStorage::taskDispatcherWork() {
  int minClientTaskCapacity = 3; //TODO: read from config
  while (m_dispatcherEnabled.load()) {
    std::vector<ClientId> notSatisfiedClients {};
    {
      std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
      for (const auto& client : m_clientTasks) {
        if (client.second.size() <= minClientTaskCapacity) {
          notSatisfiedClients.emplace_back(client.first);
        }
      }
    }

    if (!notSatisfiedClients.empty()) {
      std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
      for (ClientId clientId : notSatisfiedClients) {
        ClientTaskQueue& clientQueue = m_clientTasks.find(clientId)->second;
        while (!m_tasks.empty() && clientQueue.size() <= minClientTaskCapacity) {
          const ClientTask& clientTask = m_tasks.front();
          m_tasks.pop_front();
          clientQueue.push_back(clientTask);
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
}

void DataStorage::markDone(ClientId clientId, TaskId taskId) {
  std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
  auto clientTasksIt = m_clientTasks.find(clientId);
  if (clientTasksIt == m_clientTasks.end()) {
    return;
  }

  ClientTaskQueue& clientTasks = clientTasksIt->second;
  if (clientTasks.size() > 0) {
    clientTasks.erase(std::find_if(clientTasks.begin(), clientTasks.end(),
                                   [=](const auto& pair) { return pair.taskId == taskId; }));
  }
}
