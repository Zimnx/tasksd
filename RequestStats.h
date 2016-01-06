#pragma once

class RequestStats {
public:
  virtual ~RequestStats() {
  }

  virtual void recordRequest() {
    ++m_reqCount;
  }

  virtual uint64_t getRequestCount() {
    return m_reqCount;
  }

private:
  uint64_t m_reqCount{0};
};
