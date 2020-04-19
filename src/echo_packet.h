#ifndef ECHO_PACKET_H
#define ECHO_PACKET_H

#include <iostream>

class EchoPacket {
public:
  enum { reply = 0, request = 8 };

  EchoPacket();
  EchoPacket(uint16_t identifier, uint16_t sequence);
  uint16_t get_type() const;
  uint16_t get_identifier() const;
  uint16_t get_sequence() const;

  friend std::ostream &operator<<(std::ostream &os, const EchoPacket &request);
  friend std::istream &operator>>(std::istream &is, EchoPacket &response);

private:
  static const size_t data_len = 4;
  static const size_t data_byte_len = 8;
  static const size_t type_code_index = 0;
  static const size_t checksum_index = 1;
  static const size_t identifier_index = 2;
  static const size_t sequence_index = 3;
  static const uint16_t type_code = 0x800;

  uint16_t header[data_len];

  uint16_t compute_checksum();
};

std::ostream &operator<<(std::ostream &os, const EchoPacket &request);
std::istream &operator>>(std::istream &is, EchoPacket &response);

#endif