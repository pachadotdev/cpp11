#include <cpp11/streams.hpp>
#include <iostream>

// Redirect cout/cerr:

// at the start of a program
// void init_global_streams() {
//   std::cout.rdbuf(&cpp11::streams::message_streambuf::instance());
//   std::cerr.rdbuf(&cpp11::streams::error_streambuf::instance());
// }

[[cpp11::register]] void streams_cout() {
  cpp11::streams::stream_redirect redirect;
  std::cout << "This is a message to R console" << std::endl;
}

[[cpp11::register]] void streams_warn() {
  cpp11::streams::stream_redirect redirect;
  std::cerr << "This becomes an R warning" << std::endl;
}

[[cpp11::register]] void streams_cerr() {
  cpp11::streams::stream_redirect redirect;
  std::cerr << "This becomes an R error" << std::endl;
}

// Use dedicated streams directly:

[[cpp11::register]] void streams_cout2() {
  cpp11::streams::message_stream() << "This is a message to R console" << std::endl;
}

[[cpp11::register]] void streams_warn2() {
  cpp11::streams::warning_stream() << "This becomes an R warning" << std::endl;
}

[[cpp11::register]] void streams_cerr2() {
  cpp11::streams::error_stream() << "This becomes an R error" << std::endl;
}
