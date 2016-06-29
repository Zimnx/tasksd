#include "PutTaskHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "RequestStats.h"
#include "DataStorage.h"

using namespace proxygen;

PutTaskHandler::PutTaskHandler(RequestStats* requestStats, IDataStorage* dataStorage)
  : m_requestStats(requestStats)
  , m_dataStorage(dataStorage)
  , m_error { false }
{}

void PutTaskHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  m_requestStats->recordRequest();
  boost::optional<HTTPMethod> method = headers->getMethod();
  if (!method || *method != HTTPMethod::POST) {
    m_error = true;
    m_errorCode = 403;
    m_errorString = "Wrong method, POST required";
  }
}

void PutTaskHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (m_body) {
    m_body->prependChain(std::move(body));
  } else {
    m_body = std::move(body);
  }
}

void PutTaskHandler::onEOM() noexcept {
  if (m_error) {
    ResponseBuilder(downstream_)
      .status(m_errorCode, m_errorString)
      .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
      .sendWithEOM();
    return;
  }

  m_dataStorage->put(IDataStorage::Task{m_body.release()});

  ResponseBuilder(downstream_)
    .status(200, "OK")
    .header("Request-Number", folly::to<std::string>(m_requestStats->getRequestCount()))
    .sendWithEOM();
}

void PutTaskHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
}

void PutTaskHandler::requestComplete() noexcept {
  delete this;
}

void PutTaskHandler::onError(ProxygenError err) noexcept {
  delete this;
}
