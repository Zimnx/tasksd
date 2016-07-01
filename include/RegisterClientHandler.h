#pragma once

#include <folly/Memory.h>
#include <utils/log/Logger.h>
#include <proxygen/httpserver/RequestHandler.h>

namespace proxygen {
class ResponseHandler;
}

class RequestStats;
class IClientHandler;

class RegisterClientHandler : public proxygen::RequestHandler {
public:
  explicit RegisterClientHandler(RequestStats* RequestStats, IClientHandler* clientHandler);

  void onRequest(std::unique_ptr<proxygen::HTTPMessage> message) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

private:
  RequestStats* const m_requestStats{nullptr};
  IClientHandler* const m_clientHandler{nullptr};

  static const std::string CLIENT_ID_HEADER;

  void setError(uint32_t code, const std::string& message);

  uint32_t m_clientId;

  bool m_error;
  std::string m_errorMessage;
  uint32_t m_errorCode;

  Logger m_logger;
};
