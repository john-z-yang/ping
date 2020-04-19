#ifndef PINGER_H
#define PINGER_H

#include <boost/asio.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>

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
    Statistics();

    void add_latency(uint16_t latency);
    void add_timeout();

    uint64_t get_total_packets_sent() const;
    double get_packet_loss() const;
    uint64_t get_min_latency() const;
    double get_average_latency() const;
    uint64_t get_max_latency() const;
    double get_mid_latency() const;

  private:
    std::multiset<uint16_t> latencies;
    uint64_t num_timeout;

    friend std::ostream &operator<<(std::ostream &os,
                                    const Statistics &statistics);
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

  uint16_t get_identifier() const;
  void start_send();
  void start_recive();
  void handle_receive(const boost::system::error_code &ec,
                      size_t bytes_transferred);
  void handle_timeout(const boost::system::error_code &ec);
};

std::ostream &operator<<(std::ostream &os,
                         const Pinger::Statistics &statistics);
#endif