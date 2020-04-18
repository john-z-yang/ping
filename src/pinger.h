#ifndef PINGER_H
#define PINGER_H
#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include "echo_packet.h"

using boost::asio::deadline_timer;
using boost::asio::ip::icmp;

class Pinger {
public:
  Pinger(boost::asio::io_service &io_service,
         boost::asio::ip::icmp::endpoint &destination,
         boost::posix_time::millisec &timeout_duration);
  Pinger(boost::asio::io_service &io_service,
         boost::asio::ip::icmp::endpoint &destination);

  class Statistics {
  public:
    std::vector<boost::posix_time::ptime> latencies;
    int64_t num_timeout;

    Statistics();
  };

private:
  const icmp::endpoint destination;

  const boost::posix_time::millisec packet_interval;
  const boost::posix_time::millisec timeout_duration;

  icmp::socket socket;
  boost::asio::streambuf reply_buffer;
  boost::asio::deadline_timer timer;
  boost::posix_time::ptime time_sent;

  bool response_recived;
  uint16_t sequence;

  Statistics statistics;

  uint16_t get_identifier();
  void start_send();
  void start_recive();
  void handle_receive(const boost::system::error_code &ec,
                      size_t bytes_transferred);
  void handle_timeout(const boost::system::error_code &ec);
};
#endif