#pragma once

#include <iostream>   // for std::cout, std::cerr
#include <ostream>    // for std::ostream
#include <streambuf>  // for std::streambuf
#include <string>     // for std::string

#include "cpp11/function.hpp"  // for message
#include "cpp11/protect.hpp"   // for stop, warning

namespace cpp11 {

namespace streams {

// Stream buffer that logs to R's console without causing errors
class cerr_streambuf : public std::streambuf {
 private:
  std::string buffer_;

 protected:
  virtual int overflow(int c) override {
    if (c != EOF) {
      buffer_ += static_cast<char>(c);
      if (c == '\n') {
        // Log to R console directly using REprintf instead of warning/stop
        // which avoids triggering R's error mechanism
        REprintf("[stderr] %s", buffer_.c_str());
        buffer_.clear();
      }
    }
    return c;
  }

  virtual std::streamsize xsputn(const char* s, std::streamsize n) override {
    buffer_.append(s, n);

    size_t pos = 0;
    while ((pos = buffer_.find('\n')) != std::string::npos) {
      REprintf("[stderr] %s\n", buffer_.substr(0, pos).c_str());
      buffer_.erase(0, pos + 1);
    }

    return n;
  }

  virtual int sync() override {
    if (!buffer_.empty()) {
      REprintf("[stderr] %s\n", buffer_.c_str());
      buffer_.clear();
    }
    return 0;
  }

 public:
  static cerr_streambuf& instance() {
    static cerr_streambuf instance;
    return instance;
  }
};

// Stream buffer that redirects output to cpp11::stop()
// This is provided for explicit error stream usage, but not used for std::cerr
class error_streambuf : public std::streambuf {
 private:
  std::string buffer_;

 protected:
  virtual int overflow(int c) override {
    if (c != EOF) {
      buffer_ += static_cast<char>(c);
      if (c == '\n') {
        cpp11::stop(buffer_);
        buffer_.clear();
      }
    }
    return c;
  }

  virtual std::streamsize xsputn(const char* s, std::streamsize n) override {
    buffer_.append(s, n);

    size_t pos = 0;
    while ((pos = buffer_.find('\n')) != std::string::npos) {
      cpp11::stop(buffer_.substr(0, pos));
      buffer_.erase(0, pos + 1);
    }

    return n;
  }

  virtual int sync() override {
    if (!buffer_.empty()) {
      cpp11::stop(buffer_);
      buffer_.clear();
    }
    return 0;
  }

 public:
  static error_streambuf& instance() {
    static error_streambuf instance;
    return instance;
  }
};

// Stream buffer that redirects output to cpp11::message()
class message_streambuf : public std::streambuf {
 private:
  std::string buffer_;

 protected:
  virtual int overflow(int c) override {
    if (c != EOF) {
      buffer_ += static_cast<char>(c);
      if (c == '\n') {
        cpp11::message(buffer_);
        buffer_.clear();
      }
    }
    return c;
  }

  virtual std::streamsize xsputn(const char* s, std::streamsize n) override {
    buffer_.append(s, n);

    size_t pos = 0;
    while ((pos = buffer_.find('\n')) != std::string::npos) {
      cpp11::message(buffer_.substr(0, pos));
      buffer_.erase(0, pos + 1);
    }

    return n;
  }

  virtual int sync() override {
    if (!buffer_.empty()) {
      cpp11::message(buffer_);
      buffer_.clear();
    }
    return 0;
  }

 public:
  static message_streambuf& instance() {
    static message_streambuf instance;
    return instance;
  }
};

// Stream buffer that redirects output to cpp11::warning()
class warning_streambuf : public std::streambuf {
 private:
  std::string buffer_;

 protected:
  virtual int overflow(int c) override {
    if (c != EOF) {
      buffer_ += static_cast<char>(c);
      if (c == '\n') {
        cpp11::warning(buffer_);
        buffer_.clear();
      }
    }
    return c;
  }

  virtual std::streamsize xsputn(const char* s, std::streamsize n) override {
    buffer_.append(s, n);

    size_t pos = 0;
    while ((pos = buffer_.find('\n')) != std::string::npos) {
      cpp11::warning(buffer_.substr(0, pos));
      buffer_.erase(0, pos + 1);
    }

    return n;
  }

  virtual int sync() override {
    if (!buffer_.empty()) {
      cpp11::warning(buffer_);
      buffer_.clear();
    }
    return 0;
  }

 public:
  static warning_streambuf& instance() {
    static warning_streambuf instance;
    return instance;
  }
};

// Get streams connected to cpp11 communication channels
inline std::ostream& error_stream() {
  static std::ostream instance(&error_streambuf::instance());
  return instance;
}

inline std::ostream& message_stream() {
  static std::ostream instance(&message_streambuf::instance());
  return instance;
}

inline std::ostream& warning_stream() {
  static std::ostream instance(&warning_streambuf::instance());
  return instance;
}

// RAII class to redirect std::cout/std::cerr and restore on destruction
class stream_redirect {
 private:
  std::streambuf* old_cout_;
  std::streambuf* old_cerr_;
  bool redirect_cout_;
  bool redirect_cerr_;

 public:
  stream_redirect(bool redirect_cout = true, bool redirect_cerr = true)
      : redirect_cout_(redirect_cout), redirect_cerr_(redirect_cerr) {
    if (redirect_cout_) {
      old_cout_ = std::cout.rdbuf(&message_streambuf::instance());
    }
    if (redirect_cerr_) {
      // Use a special cerr buffer that uses REprintf directly instead of R's
      // error/warning system
      old_cerr_ = std::cerr.rdbuf(&cerr_streambuf::instance());
    }
  }

  ~stream_redirect() {
    try {
      if (redirect_cout_) {
        std::cout.rdbuf(old_cout_);
      }
      if (redirect_cerr_) {
        std::cerr.rdbuf(old_cerr_);
      }
    } catch (...) {
      // Prevent exceptions from propagating out of the destructor
    }
  }

  // Prevent copying
  stream_redirect(const stream_redirect&) = delete;
  stream_redirect& operator=(const stream_redirect&) = delete;
};

}  // namespace streams

}  // namespace cpp11
