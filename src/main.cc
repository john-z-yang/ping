#include <cxxopts.hpp>
#include <iostream>

#include "pinger.h"

const int max_timeout = 10000;
const int min_timeout = 100;

bool is_valid_timeout(int milliseconds) {
  return milliseconds >= min_timeout && milliseconds <= max_timeout;
}

asio::ip::icmp::endpoint resolve_endpint(asio::io_service &io_service,
                                         std::string address) {
  const icmp::resolver::query query(icmp::v4(), address, "");
  icmp::resolver resolver(io_service);
  return *resolver.resolve(query);
}

cxxopts::ParseResult parse_options(cxxopts::Options &options, int &argc,
                                   char *argv[]) {
  const std::string defualt_timeout = "5000";
  options.add_options()("h,help", "Print usage")(
      "t,timeout", "Number of milliseconds before a request times out",
      cxxopts::value<int>()->default_value(defualt_timeout));
  options.custom_help("[OPTION...] destination");

  cxxopts::ParseResult options_result = options.parse(argc, argv);

  if (options_result.count("help")) {
    std::cout << options.help() << std::endl
              << "You may need to run this program as root";
    exit(0);
  }

  if (options_result.count("timeout") &&
      !is_valid_timeout(options_result["timeout"].as<int>())) {
    std::cerr << "timeout must be within [" << min_timeout << ", "
              << max_timeout << "]" << std::endl;
    exit(1);
  }

  return options_result;
}

asio::ip::icmp::endpoint parse_destination(asio::io_service &io_service,
                                           int &argc, char *argv[]) {
  if (argc == 1) {
    std::cerr << "Missing ip address or domain name, see --help for more info"
              << std::endl;
    exit(1);
  }

  try {
    return resolve_endpint(io_service, argv[1]);
  } catch (const asio::error_code &ec) {
    std::cerr << "Unable to resolve endpoint: " << argv[1] << std::endl;
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  cxxopts::Options options("Ping",
                           "Send/recive icmp echo packets to/from an endpoint");
  cxxopts::ParseResult options_result = parse_options(options, argc, argv);

  asio::io_service io_service;

  asio::ip::icmp::endpoint endpoint = parse_destination(io_service, argc, argv);

  asio::chrono::milliseconds timeout_duration =
      asio::chrono::milliseconds(options_result["timeout"].as<int>());

  try {
    Pinger pinger(io_service, endpoint, timeout_duration);
    io_service.run();
  } catch (const asio::system_error &se) {
    std::cerr << "ERROR: " << se.what() << std::endl
              << "You may need to run this program as root" << std::endl;
  }

  return 0;
}