#include "UnregisterClientHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "RequestStats.h"
#include "IClientHandler.h"

using namespace proxygen;

const std::string UnregisterClientHandler::CLIENT_ID_HEADER("ClientId");

UnregisterClientHandler::UnregisterClientHandler(RequestStats* requestStats, IClientHandler* clientHandler)
  : m_requestStats(requestStats)
  , m_clientHandler(clientHandler)
  , m_error { false }
{}

void UnregisterClientHandler::onRequest(std::unique_ptr<HTTPMessage> message) noexcept {
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

  m_clientId = folly::to<uint32_t>(headers.getSingleOrEmpty(CLIENT_ID_HEADER));
}

void UnregisterClientHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {
}

void UnregisterClientHandler::onEOM() noexcept {
  if (m_error) {
    ResponseBuilder(downstream_)
      .status(m_errorCode, m_errorMessage)
      .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
      .sendWithEOM();
    return;
  }

  m_clientHandler->unregisterClient(m_clientId);

  ResponseBuilder(downstream_)
    .status(200, "OK")
    .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
    .sendWithEOM();
}

void UnregisterClientHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
}

void UnregisterClientHandler::requestComplete() noexcept {
  delete this;
}

void UnregisterClientHandler::onError(ProxygenError err) noexcept {
  delete this;
}

void UnregisterClientHandler::setError(uint32_t code, std::string message) {
  m_error = true;
  m_errorCode = code;
  m_errorMessage = message;
}
