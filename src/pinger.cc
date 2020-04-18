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

uint16_t Pinger::get_identifier() {
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
    boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::universal_time();
    std::cout << "icmp_seq=" << response.get_sequence()
              << ", time=" << (now - time_sent).total_milliseconds() << " ms"
              << std::endl;
    response_recived = true;
    ++sequence;

    timer.expires_at(now + packet_interval);
    timer.wait();
  }

  start_recive();
}

void Pinger::handle_timeout(const boost::system::error_code &ec) {
  if (!response_recived) {
    std::cout << "Request timed out" << std::endl;
  }
  start_send();
}

int main() {
  boost::asio::io_service io_service;

  icmp::resolver::query query(icmp::v4(), "google.com", "");
  icmp::resolver resolver(io_service);

  boost::asio::ip::icmp::endpoint destination = *resolver.resolve(query);
  boost::posix_time::millisec timeout_duration =
      boost::posix_time::millisec(3000);

  Pinger pinger(io_service, destination, timeout_duration);

  io_service.run();

  return 0;
}