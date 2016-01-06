#include "EchoHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "RequestStats.h"

using namespace proxygen;

EchoHandler::EchoHandler(RequestStats* RequestStats) : m_RequestStats(RequestStats) {
}

void EchoHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  m_RequestStats->recordRequest();
}

void EchoHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (m_body) {
    m_body->prependChain(std::move(body));
  } else {
    m_body = std::move(body);
  }
}

void EchoHandler::onEOM() noexcept {
  ResponseBuilder(downstream_)
    .status(200, "OK")
    .header("Request-Number", folly::to<std::string>(m_RequestStats->getRequestCount()))
    .body(std::move(m_body))
    .sendWithEOM();
}

void EchoHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
}

void EchoHandler::requestComplete() noexcept {
  delete this;
}

void EchoHandler::onError(ProxygenError err) noexcept {
  delete this;
}
