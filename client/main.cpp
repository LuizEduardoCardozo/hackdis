#include <cstdint>
#include <cstdlib>
#include <string>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

#include "../common/logger/Logger.h"
#include "../common/socket_io/read.h"
#include "../common/socket_io/write.h"

const int PORT = 5643;
const int ADDR = INADDR_LOOPBACK;

const size_t HEADER_SIZE = 4;
const size_t MESSAGE_SIZE = 4096;
const size_t NULLBYTE_SIZE = 1;

int query( int socked_connection_fd, const char* msg )
{
  int write_error, read_error;
  size_t msg_len;

  msg_len = strlen( msg );
  if ( msg_len > MESSAGE_SIZE )
    return -1;

  char send_message_buffer[ HEADER_SIZE + MESSAGE_SIZE ];
  memcpy( send_message_buffer, &msg_len, HEADER_SIZE );
  memcpy( &send_message_buffer[ HEADER_SIZE ], msg, msg_len );

  write_error = write_a( socked_connection_fd, send_message_buffer, HEADER_SIZE + msg_len );
  if ( write_error )
  {
    return write_error;
  }

  Logger::Info("CLIENT", "Message sent to the server | " + std::string(&send_message_buffer[HEADER_SIZE]));

  char response_buffer[ HEADER_SIZE + MESSAGE_SIZE + NULLBYTE_SIZE ];
  errno = 0;
  read_error = read_a(socked_connection_fd, response_buffer, HEADER_SIZE);
  if(read_error)
  {
    switch(errno)
    {
    case 0:
      Logger::Error("CLIENT", "EOF");
      break;
    default:
      Logger::Error("CLIENT", "error while reading the socket");
      break;
    }
  }

  memcpy(&msg_len, response_buffer, HEADER_SIZE);

  if(msg_len > MESSAGE_SIZE)
  {
    Logger::Error("CLIENT", "message too long");
    return -1;
  }

  read_error = read_a(socked_connection_fd, &response_buffer[HEADER_SIZE], msg_len);
  if(read_error)
  {
    Logger::Info("CLIENT", "read_error: " + std::to_string(read_error));
    Logger::Error("CLIENT", "error while reading the socket");
    return read_error;
  }

  response_buffer[HEADER_SIZE + msg_len] = '\0';

  Logger::Info("CLIENT", "Message received from server | " + std::string(&response_buffer[HEADER_SIZE]));

  return 0;
}

int main()
{
  struct sockaddr_in addr {
  };
  // int result;

  Logger logger( "CLIENT" );

  int connection_socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( connection_socket_fd == -1 )
  {
    logger.Error( "error while creating the socket" );
    exit( 1 );
  }

  addr.sin_family = AF_INET;
  addr.sin_port = ntohs( PORT );
  addr.sin_addr.s_addr = ntohl( ADDR );

  int rv = connect( connection_socket_fd, (const struct sockaddr*)&addr, sizeof( addr ) );
  if ( rv == -1 )
  {
    logger.Error( "error while creating the connection with the socket" );
    exit( 1 );
  }

  char msg[] = "Hello";
  int err = query(connection_socket_fd, msg);
  if(err)
  {
   logger.Error("error while query the message");
   exit(1);
  }

  close(connection_socket_fd);
}