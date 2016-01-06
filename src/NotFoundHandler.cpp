#include "NotFoundHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "RequestStats.h"

using namespace proxygen;

NotFoundHandler::NotFoundHandler(RequestStats* RequestStats) : m_RequestStats(RequestStats) {
}

void NotFoundHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  m_RequestStats->recordRequest();
}

void NotFoundHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {
}

void NotFoundHandler::onEOM() noexcept {
  ResponseBuilder(downstream_)
    .status(404, "Not Found")
    .header("Request-Number", folly::to<std::string>(m_RequestStats->getRequestCount()))
    .body("404 Not Found")
    .sendWithEOM();
}

void NotFoundHandler::onUpgrade(UpgradeProtocol) noexcept {
}

void NotFoundHandler::requestComplete() noexcept {
  delete this;
}

void NotFoundHandler::onError(ProxygenError err) noexcept {
  delete this;
}
