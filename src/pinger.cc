#include "pinger.h"

Pinger::Pinger(asio::io_service &io_service,
               const asio::ip::icmp::endpoint &destination,
               const asio::chrono::milliseconds &timeout_duration)
    : destination(destination), packet_interval(1000),
      timeout_duration(timeout_duration), socket(io_service, icmp::v4()),
      timer(io_service), sequence(0) {
  start_send();
  start_recive();
}

Pinger::Pinger(asio::io_service &io_service,
               const asio::ip::icmp::endpoint &destination)
    : destination(destination), packet_interval(1000), timeout_duration(5000),
      socket(io_service, icmp::v4()), timer(io_service), sequence(0) {
  start_send();
  start_recive();
}

Pinger::Statistics::Statistics() : num_timeout(0) {}

void Pinger::Statistics::add_latency(uint16_t latency) {
  latencies.insert(latency);
}

void Pinger::Statistics::increment_timeout() { ++num_timeout; }

uint64_t Pinger::Statistics::get_total_packets_sent() const {
  return latencies.size() + num_timeout;
}

double Pinger::Statistics::get_packet_loss() const {
  if (get_total_packets_sent() == 0) {
    return 0;
  }
  return static_cast<double>(num_timeout) /
         static_cast<double>(latencies.size() + num_timeout);
}

uint64_t Pinger::Statistics::get_min_latency() const {
  return latencies.empty() ? 0 : *latencies.begin();
}

uint64_t Pinger::Statistics::get_average_latency() const {
  if (get_total_packets_sent() == 0) {
    return 0;
  }
  const int64_t total_latencies =
      std::accumulate(latencies.begin(), latencies.end(), 0);
  return total_latencies / latencies.size();
}

uint64_t Pinger::Statistics::get_max_latency() const {
  return latencies.empty() ? 0 : *std::prev(latencies.end());
}

double Pinger::Statistics::to_milliseconds(uint64_t microseconds) {
  const double microseconds_per_millisecond = 1000;
  return static_cast<double>(microseconds) / microseconds_per_millisecond;
}

std::ostream &operator<<(std::ostream &os,
                         const Pinger::Statistics &statistics) {
  const uint8_t precision = 3;
  os << std::fixed << std::showpoint << std::setprecision(precision);

  os << statistics.get_total_packets_sent() << " packets transmitted, "
     << statistics.get_packet_loss() << "% loss" << std::endl
     << "min/avg/max/ = "
     << statistics.to_milliseconds(statistics.get_min_latency()) << "/"
     << statistics.to_milliseconds(statistics.get_average_latency()) << "/"
     << statistics.to_milliseconds(statistics.get_max_latency()) << " ms";
  return os;
}

uint16_t Pinger::get_identifier() const {
  return static_cast<unsigned short>(getpid());
}

void Pinger::start_send() {
  EchoPacket request(get_identifier(), sequence);
  asio::streambuf request_buffer;
  std::ostream os(&request_buffer);
  os << request;

  socket.send_to(request_buffer.data(), destination);

  time_sent = asio::steady_timer::clock_type::now();

  timer.expires_at(time_sent + timeout_duration);
  timer.async_wait([&](const asio::error_code &ec) { handle_timeout(ec); });

  response_recived = false;
}

void Pinger::start_recive() {
  reply_buffer.consume(reply_buffer.size());
  socket.async_receive(
      reply_buffer.prepare(65536),
      [&](const asio::error_code &ec, size_t bytes_transferred) {
        handle_receive(ec, bytes_transferred);
      });
}

void Pinger::handle_receive(const asio::error_code &ec,
                            size_t bytes_transferred) {
  reply_buffer.commit(bytes_transferred);

  std::istream is(&reply_buffer);

  EchoPacket response;
  is >> response;

  if (is && response.get_type() == EchoPacket::reply &&
      response.get_identifier() == get_identifier() &&
      response.get_sequence() == sequence) {
    const asio::chrono::steady_clock::time_point now =
        asio::chrono::steady_clock::now();
    const uint64_t latency =
        asio::chrono::duration_cast<asio::chrono::microseconds>(now - time_sent)
            .count();

    response_recived = true;
    ++sequence;

    statistics.add_latency(latency);

    std::cout << destination << ": seq=" << response.get_sequence()
              << ", time=" << statistics.to_milliseconds(latency)
              << " ms:" << std::endl
              << statistics << std::endl
              << std::endl;

    timer.expires_at(now + packet_interval);
    timer.wait();
  }

  start_recive();
}

void Pinger::handle_timeout(const asio::error_code &ec) {
  if (!response_recived) {
    std::cout << destination << ": Request timed out (>"
              << timeout_duration.count() << " ms)" << std::endl
              << std::endl;

    statistics.increment_timeout();
    response_recived = true;
    ++sequence;

    timer.expires_at(asio::chrono::steady_clock::now() + packet_interval);
    timer.wait();
  }
  start_send();
}