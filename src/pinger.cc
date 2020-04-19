#include "pinger.h"

Pinger::Pinger(boost::asio::io_service &io_service,
               boost::asio::ip::icmp::endpoint &destination,
               boost::posix_time::millisec &timeout_duration)
    : socket(io_service, icmp::v4()), destination(destination),
      timeout_duration(timeout_duration), timer(io_service), sequence(0),
      packet_interval(1000) {
  start_send();
  start_recive();
}

Pinger::Pinger(boost::asio::io_service &io_service,
               boost::asio::ip::icmp::endpoint &destination)
    : socket(io_service, icmp::v4()), destination(destination),
      timeout_duration(5000), timer(io_service), sequence(0),
      packet_interval(1000) {
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

uint64_t Pinger::Statistics::get_mid_latency() const {
  if (latencies.empty()) {
    return 0;
  } else if (latencies.size() % 2 == 0) {
    auto it1 = latencies.begin();
    auto it2 = latencies.begin();
    std::advance(it1, latencies.size() / 2 - 1);
    std::advance(it2, latencies.size() / 2);
    return *it1 + *it2 / 2;
  }
  auto it = latencies.begin();
  std::advance(it, latencies.size() / 2);
  return *it;
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
     << "min/avg/max/mdev = "
     << statistics.to_milliseconds(statistics.get_min_latency()) << "/"
     << statistics.to_milliseconds(statistics.get_average_latency()) << "/"
     << statistics.to_milliseconds(statistics.get_max_latency()) << "/"
     << statistics.to_milliseconds(statistics.get_mid_latency()) << " ms";
}

uint16_t Pinger::get_identifier() const {
  return static_cast<unsigned short>(getpid());
}

void Pinger::start_send() {
  EchoPacket request(get_identifier(), sequence);
  boost::asio::streambuf request_buffer;
  std::ostream os(&request_buffer);
  os << request;

  socket.send_to(request_buffer.data(), destination);

  time_sent = boost::posix_time::microsec_clock::universal_time();

  timer.expires_at(time_sent + timeout_duration);
  timer.async_wait(
      [&](const boost::system::error_code &ec) { handle_timeout(ec); });

  response_recived = false;
}

void Pinger::start_recive() {
  reply_buffer.consume(reply_buffer.size());
  socket.async_receive(
      reply_buffer.prepare(65536),
      [&](const boost::system::error_code &ec, size_t bytes_transferred) {
        handle_receive(ec, bytes_transferred);
      });
}

void Pinger::handle_receive(const boost::system::error_code &ec,
                            size_t bytes_transferred) {
  reply_buffer.commit(bytes_transferred);

  std::istream is(&reply_buffer);

  EchoPacket response;
  is >> response;

  if (is && response.get_type() == EchoPacket::reply &&
      response.get_identifier() == get_identifier() &&
      response.get_sequence() == sequence) {
    const boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::universal_time();
    const uint64_t latency = (now - time_sent).total_microseconds();

    response_recived = true;
    ++sequence;

    statistics.add_latency(latency);

    std::cout << destination << ":"
              << " seq=" << response.get_sequence()
              << ", time=" << statistics.to_milliseconds(latency)
              << "ms:" << std::endl
              << statistics << std::endl
              << std::endl;

    timer.expires_at(now + packet_interval);
    timer.wait();
  }

  start_recive();
}

void Pinger::handle_timeout(const boost::system::error_code &ec) {
  if (!response_recived) {
    std::cout << destination << ": Request timed out" << std::endl;
    statistics.increment_timeout();
  }
  start_send();
}

int main(int argc, char *argv[]) {
  boost::asio::io_service io_service;

  icmp::resolver::query query(icmp::v4(), argv[1], "");
  icmp::resolver resolver(io_service);

  boost::asio::ip::icmp::endpoint destination = *resolver.resolve(query);
  boost::posix_time::millisec timeout_duration =
      boost::posix_time::millisec(3000);

  Pinger pinger(io_service, destination, timeout_duration);

  io_service.run();

  return 0;
}