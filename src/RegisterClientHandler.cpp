#include "RegisterClientHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "RequestStats.h"
#include "IClientHandler.h"

using namespace proxygen;

const std::string RegisterClientHandler::CLIENT_ID_HEADER("ClientId");

RegisterClientHandler::RegisterClientHandler(RequestStats* requestStats, IClientHandler* clientHandler)
  : m_requestStats(requestStats)
  , m_clientHandler(clientHandler)
  , m_error { false }
  , m_logger {}
{}

void RegisterClientHandler::onRequest(std::unique_ptr<HTTPMessage> message) noexcept {
  m_requestStats->recordRequest();
  boost::optional<HTTPMethod> method = message->getMethod();
  if (!method || *method != HTTPMethod::POST) {
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

void RegisterClientHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {
}

void RegisterClientHandler::onEOM() noexcept {
  if (m_error) {
    ResponseBuilder(downstream_)
      .status(m_errorCode, m_errorMessage)
      .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
      .sendWithEOM();
    return;
  }

  m_clientHandler->registerClient(m_clientId);
  log_debug(m_logger, "Registered new client %d", m_clientId);

  ResponseBuilder(downstream_)
    .status(200, "OK")
    .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
    .sendWithEOM();
}

void RegisterClientHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
}

void RegisterClientHandler::requestComplete() noexcept {
  delete this;
}

void RegisterClientHandler::onError(ProxygenError err) noexcept {
  delete this;
}

void RegisterClientHandler::setError(uint32_t code, const std::string& message) {
  log_error(m_logger, "Error during register, code: %d, message: %s", code, message.c_str());
  m_error = true;
  m_errorCode = code;
  m_errorMessage = message;
}
