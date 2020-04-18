#include "echo_packet.h"

EchoPacket::EchoPacket() {}

EchoPacket::EchoPacket(uint16_t identifier, uint16_t sequence) {
  data[type_code_index] = type_code;
  data[identifier_index] = identifier;
  data[sequence_index] = sequence;
  data[checksum_index] = compute_checksum();
}

uint16_t EchoPacket::get_type() { return data[type_code_index] >> 8; }

uint16_t EchoPacket::get_identifier() { return data[identifier_index]; }

uint16_t EchoPacket::get_sequence() { return data[sequence_index]; }

uint16_t EchoPacket::compute_checksum() {
  uint32_t sum =
      data[type_code_index] + data[identifier_index] + data[sequence_index];
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);

  return static_cast<uint16_t>(~sum);
}

std::ostream &operator<<(std::ostream &os, const EchoPacket &request) {
  uint8_t bytes[request.data_byte_len];
  for (size_t i = 0; i < request.data_len; i++) {
    bytes[i * 2] = request.data[i] >> 8;
    bytes[i * 2 + 1] = request.data[i] & 0xFF;
  }
  return os.write(reinterpret_cast<const char *>(bytes), 8);
}

std::istream &operator>>(std::istream &is, EchoPacket &response) {
  const size_t bytes_per_word = 4;
  const size_t ipv4_header_length = (is.get() & 0xF) * bytes_per_word;

  is.ignore(ipv4_header_length - 1, std::char_traits<char>::eof());

  uint8_t bytes[response.data_byte_len];
  is.read(reinterpret_cast<char *>(bytes), response.data_byte_len);
  for (size_t i = 0; i < response.data_len; i++) {
    response.data[i] = (bytes[i * 2] << 8) + bytes[i * 2 + 1];
  }
  return is;
}