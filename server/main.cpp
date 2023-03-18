#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "../common/logger/Logger.h"
#include "../common/socket_io/read.h"
#include "../common/socket_io/write.h"

const int HEADER_SIZE = 4;
const int NULLBYTE_SIZE = 1;

const int PORT = 5643;
const int ADDR = 0;
const size_t MESSAGE_SIZE = 4096;

int8_t handle_request(int socket_connection_fd)
{
  char read_buffer[ HEADER_SIZE + MESSAGE_SIZE + NULLBYTE_SIZE ];
  int8_t read_error, write_error;

  errno = 0;

  read_error = read_a( socket_connection_fd, read_buffer, HEADER_SIZE );
  if ( read_error )
  {
    if(errno == 0)
    {
      Logger::Error( "SERVER", "EOF" );
    } else {
      Logger::Error( "SERVER", "read error" );
    }

    return read_error;
  }

  size_t payload_len = 0;
  memcpy( &payload_len, read_buffer, HEADER_SIZE );
  if ( payload_len > MESSAGE_SIZE )
  {
    Logger::Error("SERVER", "received message is too long");
  }

  read_error = read_a(socket_connection_fd, &read_buffer[HEADER_SIZE], payload_len);
  if(read_error == -1)
  {
    Logger::Error("SERVER", "error while reading the message");
  }

  read_buffer[HEADER_SIZE + payload_len] = '\0';
  Logger::Info("SERVER", std::string("Client said: ") + std::string(&read_buffer[HEADER_SIZE]));

  const char reply[] = "Server response";

  size_t reply_size = sizeof(reply);
  size_t payload_size = HEADER_SIZE + reply_size;

  char response_buffer[payload_size];

  memcpy(response_buffer, &reply_size, HEADER_SIZE);
  memcpy(&response_buffer[HEADER_SIZE], reply, payload_size);

  write_error = write_a(socket_connection_fd, response_buffer, payload_size);

  return write_error;
}


int main()
{
  Logger logger( "SERVER" );

  int opt = 1, new_connection_fd, rv; //result;
  struct sockaddr_in addr {
  };
  struct sockaddr_in client_addr {
  };

  logger.Info( "Creating a new socket" );

  int socket_connection_fd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( socket_connection_fd == -1 )
  {
    logger.Error( "error while creating the socket" );
    exit(1);
  }

  setsockopt( socket_connection_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) );

  addr.sin_family = AF_INET;
  addr.sin_port = ntohs( PORT );
  addr.sin_addr.s_addr = ntohl( ADDR );

  rv = bind( socket_connection_fd, (const sockaddr*)&addr, sizeof( addr ) );
  if ( rv == -1 )
  {
    logger.Error( "error while binging the socket to the address" );
    exit(1);
  }

  rv = listen( socket_connection_fd, SOMAXCONN );
  if ( rv == -1 )
  {
    logger.Error( "error while listening with the socket" );
    exit(1);
  }

  logger.Info("awaiting for connections");

  while ( true )
  {
    socklen_t socklen = sizeof( client_addr );

    new_connection_fd = accept( socket_connection_fd, (struct sockaddr*)&client_addr, &socklen );
    if ( new_connection_fd == -1 )
    {
      continue;
    }

      int8_t err = handle_request(new_connection_fd);
      if (err) break;
  }
}