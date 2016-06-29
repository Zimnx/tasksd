#pragma once

#include <folly/Memory.h>
#include <proxygen/httpserver/RequestHandler.h>

namespace proxygen {
class ResponseHandler;
}

class RequestStats;
class IDataStorage;

class PutTaskHandler : public proxygen::RequestHandler {
public:
  explicit PutTaskHandler(RequestStats* RequestStats, IDataStorage* dataStorage);

  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

private:
  RequestStats* const m_requestStats{nullptr};
  IDataStorage* const m_dataStorage{nullptr};

  std::unique_ptr<folly::IOBuf> m_body;
  bool m_error;
  std::string m_errorString;
  uint32_t m_errorCode;
};
