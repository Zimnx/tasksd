#pragma once
#include <proxygen/httpserver/RequestHandlerFactory.h>

#include "RequestStats.h"

class HandlersFactory : public proxygen::RequestHandlerFactory {
public:
  void onServerStart(folly::EventBase*) noexcept override {
    m_RequestStats.reset(new RequestStats);
  }

  void onServerStop() noexcept override {
    m_RequestStats.reset();
  }

  proxygen::RequestHandler* onRequest(proxygen::RequestHandler*, proxygen::HTTPMessage* request) noexcept override;
  template <typename T, typename... Args>
  void registerHandler(std::string resource, Args&&... args);

private:
  std::vector<std::string> splitPath(std::string path) noexcept;

  folly::ThreadLocalPtr<RequestStats> m_RequestStats;
  std::map<std::vector<std::string>, std::function<RequestHandler*()>> m_handlers;
};
