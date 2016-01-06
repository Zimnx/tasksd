#include <gflags/gflags.h>
#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
#include <functional>

#include "EchoHandler.h"
#include "NotFoundHandler.h"
#include "RequestStats.h"

using namespace proxygen;

using folly::EventBase;
using folly::EventBaseManager;
using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;

DEFINE_int32(http_port, 11000, "Port to listen on with HTTP protocol");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_int32(threads, 0, "Number of threads to listen on. Numbers <= 0 will use the number of cores on this machine.");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::vector<HTTPServer::IPConfig> IPs = {{SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP}};

  if (FLAGS_threads <= 0) {
    FLAGS_threads = std::thread::hardware_concurrency();
    CHECK(FLAGS_threads > 0);
  }

  auto handlersFactory = std::make_unique<HandlersFactory>();
  handlersFactory->registerHandler<EchoHandler>("echo");

  HTTPServerOptions options;
  options.threads = static_cast<size_t>(FLAGS_threads);
  options.idleTimeout = std::chrono::milliseconds(60000);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.enableContentCompression = true;
  options.handlerFactories = RequestHandlerChain().addThen(std::move(handlersFactory)).build();

  HTTPServer server(std::move(options));
  server.bind(IPs);

  std::thread t([&]() { server.start(); });

  t.join();
  return 0;
}