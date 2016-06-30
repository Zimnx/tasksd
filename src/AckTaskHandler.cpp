#include "AckTaskHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "RequestStats.h"
#include "IDataStorage.h"

using namespace proxygen;

const std::string AckTaskHandler::CLIENT_ID_HEADER("ClientId");
const std::string AckTaskHandler::TASK_ID_HEADER("TaskId");

AckTaskHandler::AckTaskHandler(RequestStats* requestStats, IDataStorage* dataStorage)
  : m_requestStats(requestStats)
  , m_dataStorage(dataStorage)
  , m_error { false }
{}

void AckTaskHandler::onRequest(std::unique_ptr<HTTPMessage> message) noexcept {
  m_requestStats->recordRequest();
  boost::optional<HTTPMethod> method = message->getMethod();
  if (!method || *method != HTTPMethod::GET) {
    setError(400, "Wrong method, POST required");
    return;
  }
  HTTPHeaders headers = message->getHeaders();
  if (!headers.exists(CLIENT_ID_HEADER)) {
    setError(400, "No " + CLIENT_ID_HEADER + " header available");
    return;
  }

  if (!headers.exists(TASK_ID_HEADER)) {
    setError(400, "No " + TASK_ID_HEADER + " header available");
  }

  m_clientId = folly::to<uint32_t>(headers.getSingleOrEmpty(CLIENT_ID_HEADER));
  m_taskId = folly::to<uint64_t>(headers.getSingleOrEmpty(TASK_ID_HEADER));
}

void AckTaskHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {
}

void AckTaskHandler::onEOM() noexcept {
  if (m_error) {
    ResponseBuilder(downstream_)
      .status(m_errorCode, m_errorMessage)
      .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
      .sendWithEOM();
    return;
  }

  m_dataStorage->markDone(m_clientId, m_taskId);

  ResponseBuilder(downstream_)
    .status(200, "OK")
    .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
    .sendWithEOM();
}

void AckTaskHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
}

void AckTaskHandler::requestComplete() noexcept {
  delete this;
}

void AckTaskHandler::onError(ProxygenError err) noexcept {
  delete this;
}

void AckTaskHandler::setError(uint32_t code, std::string message) {
  m_error = true;
  m_errorCode = code;
  m_errorMessage = message;
}
