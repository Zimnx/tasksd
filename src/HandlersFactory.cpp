#include <boost/algorithm/string.hpp>

#include "HandlersFactory.h"
#include "NotFoundHandler.h"

using namespace proxygen;

RequestHandler* HandlersFactory::onRequest(RequestHandler*, HTTPMessage* request) noexcept {
  auto pathParts = splitPath(request->getPath());
  auto comparator = [&](auto pair) {
    return pair.first.size() == pathParts.size() &&
           std::equal(std::begin(pair.first), std::end(pair.first), std::begin(pathParts));
  };
  auto handlerIt = std::find_if(m_handlers.begin(), m_handlers.end(), comparator);
  if (handlerIt != m_handlers.end()) {
    return handlerIt->second();
  } else {
    return new NotFoundHandler(m_requestStats.get());
  }
}

std::vector<std::string> HandlersFactory::splitPath(std::string path) noexcept {
  std::vector<std::string> pathParts;
  boost::trim_if(path, boost::is_any_of("/"));
  boost::split(pathParts, path, boost::algorithm::is_any_of("/"));
  return pathParts;
}
