#ifndef HACKDIS_CONNECTION_H
#define HACKDIS_CONNECTION_H

#include <cstdio>
#include <cstdint>

const size_t HEADER_SIZE = 4;
const size_t MAX_MSG_SIZE = 4096;

constexpr size_t MSG_BUFFER_SIZE = HEADER_SIZE + MAX_MSG_SIZE;

class Connection
{
public:
  /*
   * REQ = Reading requests
   * RES = Sending responses
   * END = Waiting to be deleted
   */
  enum STATE
  {
    REQ,
    RES,
    END,
  };

  int fd = 1;
  STATE state = STATE::REQ;

  size_t read_buffer_size = 0;
  uint8_t read_buffer[MSG_BUFFER_SIZE];

  size_t write_buffer_size = 0;
  size_t write_buffer[MSG_BUFFER_SIZE];

  bool write_buffer_sent = false;
};

#endif