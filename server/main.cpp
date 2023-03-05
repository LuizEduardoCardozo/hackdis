#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "../common/logger/Logger.h"

const int PORT = 5643;
const int ADDR = 0;
const size_t MESSAGE_SIZE = 64;

int main()
{
  Logger logger( "SERVER" );

  int opt = 1, new_connection_fd, rv, result;
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

    char client_message_buffer[ MESSAGE_SIZE ] = {};
    result = static_cast<int>( read( new_connection_fd, client_message_buffer, MESSAGE_SIZE - 1 ) );
    if ( result == -1 )
    {
      logger.Error( "read() error" );
      exit( 1 );
    }

    logger.Info( "message received successfully" );

    logger.Info( std::string( "client says: " ) + std::string( client_message_buffer ) );

    char server_sending_message[] = "world";
    write( new_connection_fd, server_sending_message, strlen( server_sending_message ) );

    close( new_connection_fd );
  }
}