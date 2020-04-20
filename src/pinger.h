#ifndef PINGER_H
#define PINGER_H

#include <asio.hpp>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>

#include "echo_packet.h"

using asio::ip::icmp;

class Pinger {
public:
  Pinger(asio::io_service &io_service,
         const asio::ip::icmp::endpoint &destination,
         const asio::chrono::milliseconds &timeout_duration);
  Pinger(asio::io_service &io_service,
         const asio::ip::icmp::endpoint &destination);

  class Statistics {
  public:
    Statistics();

    void add_latency(uint16_t latency);
    void increment_timeout();

    uint64_t get_total_packets_sent() const;
    double get_packet_loss() const;
    uint64_t get_min_latency() const;
    uint64_t get_average_latency() const;
    uint64_t get_max_latency() const;

    static double to_milliseconds(uint64_t microseconds);

    friend std::ostream &operator<<(std::ostream &os,
                                    const Statistics &statistics);

  private:
    std::multiset<uint64_t> latencies;
    uint64_t num_timeout;
  };

private:
  const icmp::endpoint destination;

  const asio::chrono::milliseconds packet_interval;
  const asio::chrono::milliseconds timeout_duration;

  icmp::socket socket;
  asio::streambuf reply_buffer;
  asio::steady_timer timer;
  asio::chrono::steady_clock::time_point time_sent;

  bool response_recived;

  uint16_t sequence;
  Statistics statistics;

  uint16_t get_identifier() const;
  void start_send();
  void start_recive();
  void handle_receive(const asio::error_code &ec, size_t bytes_transferred);
  void handle_timeout(const asio::error_code &ec);
};

std::ostream &operator<<(std::ostream &os,
                         const Pinger::Statistics &statistics);
#endif