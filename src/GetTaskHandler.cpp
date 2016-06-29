#include "GetTaskHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "RequestStats.h"
#include "DataStorage.h"

using namespace proxygen;

const std::string GetTaskHandler::CLIENT_ID_HEADER("ClientId");

GetTaskHandler::GetTaskHandler(RequestStats* requestStats, IDataStorage* dataStorage)
  : m_requestStats(requestStats)
  , m_dataStorage(dataStorage)
  , m_error { false }
{}

void GetTaskHandler::onRequest(std::unique_ptr<HTTPMessage> message) noexcept {
  m_requestStats->recordRequest();
  boost::optional<HTTPMethod> method = message->getMethod();
  if (!method || *method != HTTPMethod::GET) {
    setError(403, "Wrong method, POST required");
    return;
  }
  HTTPHeaders headers = message->getHeaders();
  if (!headers.exists(CLIENT_ID_HEADER)) {
    setError(403, "No " + CLIENT_ID_HEADER + " header available");
    return;
  }

  m_clientId = folly::to<uint32_t>(headers.getSingleOrEmpty(CLIENT_ID_HEADER));
}

void GetTaskHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {
}

void GetTaskHandler::onEOM() noexcept {
  if (m_error) {
    ResponseBuilder(downstream_)
      .status(m_errorCode, m_errorMessage)
      .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
      .sendWithEOM();
    return;
  }

  IDataStorage::ClientTask task = m_dataStorage->get(m_clientId);

  ResponseBuilder(downstream_)
    .status(200, "OK")
    .body(std::unique_ptr<folly::IOBuf>(task.task.get())) // This will double free buffer, change .task to unique_ptr
    .header("TaskId", folly::to<std::string>(task.taskId))
    .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
    .sendWithEOM();
}

void GetTaskHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
}

void GetTaskHandler::requestComplete() noexcept {
  delete this;
}

void GetTaskHandler::onError(ProxygenError err) noexcept {
  delete this;
}

void GetTaskHandler::setError(uint32_t code, std::string message) {
  m_error = true;
  m_errorCode = code;
  m_errorMessage = message;
}
