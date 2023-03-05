#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common/logger/Logger.h"

const int PORT = 5643;
const int ADDR = INADDR_LOOPBACK;
const size_t MESSAGE_SIZE = 64;

int main()
{
  struct sockaddr_in addr {
  };
  int result;

  Logger logger( "CLIENT" );

  int connection_socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( connection_socket_fd == -1 )
  {
    logger.Error( "error while creating the socket" );
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = ntohs( PORT );
  addr.sin_addr.s_addr = ntohl( ADDR );

  int rv = connect( connection_socket_fd, (const struct sockaddr*)&addr, sizeof( addr ) );
  if ( rv == -1 )
  {
    logger.Error( "error while creating the connection with the socket" );
    exit(1);
  }

  char msg[] = "hello";
  write( connection_socket_fd, msg, strlen( msg ) );

  char received_msg_from_server[ MESSAGE_SIZE ] = {};
  result =
    static_cast<int>( read( connection_socket_fd, received_msg_from_server, sizeof( received_msg_from_server ) - 1 ) );
  if ( result == -1 )
  {
    logger.Error( "error while receiving the message from the server" );
    exit(1);
  }

  logger.Info( "message received successfully" );

  logger.Info( std::string( "server says: " ) + std::string( received_msg_from_server ) );

  close( connection_socket_fd );
}