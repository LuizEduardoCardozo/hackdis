#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <arpa/inet.h>
#include <cassert>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>

#include "../common/Connection.h"
#include "../common/logger/Logger.h"
#include "../common/socket_io/read.h"
#include "../common/socket_io/write.h"

const int NULLBYTE_SIZE = 1;

const int PORT = 5643;
const int ADDR = 0;
const size_t MESSAGE_SIZE = 4096;

void state_req(Connection *conn);
void state_res(Connection *conn);

void fb_set_nb(int connection_fd)
{
  errno = 0;
  int flags = fcntl(connection_fd, F_GETFL, 0);
  if (errno)
  {
    Logger::Error("SERVER", "fcntl error");
    return;
  }

  flags |= O_NONBLOCK;

  errno = 0;
  fcntl(connection_fd, F_SETFL, flags);
  if (errno)
  {
    Logger::Error("SERVER", "fcntl error");
    return;
  }
}

static void put_connection_to_list(std::vector<Connection *> &connections, Connection *connection)
{
  if(connections.size() <= (size_t)connection->fd) {
    connections.resize(connection->fd + 1);
  }
  connections[connection->fd] = connection;
}

static int accept_new_connection(std::vector<Connection *> &connections, int connection_fd)
{
  struct sockaddr_in client_addr{};
  socklen_t sock_len = sizeof(client_addr);

  int conn_fd = accept(connection_fd, (struct sockaddr *)&client_addr, &sock_len);
  if (conn_fd < 0)
  {
    Logger::Error("SERVER", "Accept error");
    return -1;
  }

  fb_set_nb(conn_fd);

  Connection conn;

  conn.fd = conn_fd;
  conn.state = Connection::REQ;
  conn.read_buffer_size = 0;
  conn.write_buffer_size = 0;
  conn.write_buffer_sent = 0;

  put_connection_to_list(connections, &conn);

  return 0;
}

static bool try_one_request(Connection *connection)
{
  if (connection->read_buffer_size < HEADER_SIZE)
  {
    return false;
  }

  uint32_t len = 0;

  connection->write_buffer_sent = 0; // TODO: remove this line

  memcpy(&len, &connection->read_buffer[0], HEADER_SIZE);
  if(len > MAX_MSG_SIZE)
  {
    Logger::Error("SERVER", "Message is too long");
    connection->state = Connection::END;
    return false;
  }

  if ( HEADER_SIZE + len > connection->read_buffer_size )
  {
    Logger::Error("SERVER", "Not enough space in buffer");
    return false;
  }

  Logger::Info("SERVER", "Client says: " + std::string(reinterpret_cast<char *>(&connection->read_buffer[HEADER_SIZE])));

  memcpy(&connection->write_buffer[0], &len, HEADER_SIZE);
  memcpy(&connection->write_buffer[HEADER_SIZE], &connection->read_buffer[HEADER_SIZE], len);

  connection->write_buffer_size = HEADER_SIZE + len;

  size_t remain = connection->read_buffer_size - HEADER_SIZE - len;
  if(remain)
  {
    memmove(connection->read_buffer, &connection->read_buffer[HEADER_SIZE + len], remain);
  }

  connection->read_buffer_size = remain;

  connection->state = Connection::REQ;

  state_res(connection);

  return connection->state == Connection::RES;
}

bool try_fill_buffer(Connection *connection)
{
  ssize_t rv = 0;

  do {
    size_t cap = sizeof(connection->read_buffer) - connection->read_buffer_size;
    rv = read(connection->fd, &connection->read_buffer[connection->read_buffer_size], cap);
  } while (rv < 0 && errno == EINTR);

  if ( rv < 0 && errno == EAGAIN )
  {
    return false;
  }

  if (rv < 0)
  {
    Logger::Error("SERVER", "Error while reading the message");
    connection->state = Connection::END;
    return false;
  }

  if (rv == 0)
  {
    if (connection->read_buffer_size > 0)
    {
      Logger::Error("SERVER", "Unexpected EOF");
    } else
    {
      Logger::Error("SERVER", "EOF");
    }

    connection->state = Connection::END;

    return false;
  }

  connection->read_buffer_size += (size_t)rv;

  while( try_one_request(connection)) {}

  return connection->state == Connection::REQ;
}

void state_req(Connection *connection)
{
  while ( try_fill_buffer(connection)) {}
}

bool try_flush_buffer(Connection *connection)
{
  ssize_t rv = 0;
  do {
    size_t remain = connection->write_buffer_size - connection->write_buffer_sent;
    rv = write(connection->fd, &connection->write_buffer[connection->write_buffer_sent], remain);
  } while( rv < 0 && errno == EINTR);

  if ( rv < 0 && errno == EAGAIN )
  {
    return false;
  }

  if (rv < 0)
  {
    Logger::Error("SERVER", "Error while writing the message");
    connection->state = Connection::END;
    return false;
  }

  connection->write_buffer_sent += (size_t)rv;

  if(connection->write_buffer_sent == connection->write_buffer_size)
  {
    connection->state = Connection::REQ;
    connection->write_buffer_size = 0;
    connection->write_buffer_sent = 0;
    return false;
  }

  return true;
}

void state_res(Connection *connection)
{
  while ( try_flush_buffer(connection)) {}
}

void connection_io(Connection *connection)
{
  if ( connection->state == Connection::REQ )
  {
    state_req( connection );
  }
  else if ( connection->state == Connection::RES )
  {
    state_res( connection );
  }
  else
  {
    assert(0);
  }
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
  if ( rv )
  {
    logger.Error( "error while binging the socket to the address" );
    exit(1);
  }

  rv = listen( socket_connection_fd, SOMAXCONN );
  if ( rv )
  {
    logger.Error( "error while listening with the socket" );
    exit(1);
  }

  logger.Info("awaiting for connections");

  std::vector<Connection *> connections;

  fb_set_nb(socket_connection_fd);

  std::vector<struct pollfd> poll_args;

  while ( true )
  {
    poll_args.clear();

    struct pollfd pdf = { socket_connection_fd, POLLIN, 0 };
    poll_args.push_back( pdf );

    for ( Connection *c : connections )
    {
      if ( !c )
      {
        continue;
      }

      struct pollfd n_pdf = {};

      n_pdf.fd = c->fd;
      n_pdf.events = ( c->state == Connection::REQ ) ? POLLIN : POLLOUT;
      n_pdf.events |= POLLERR;

      poll_args.push_back( n_pdf );
    }

    rv = poll( poll_args.data(), (nfds_t)poll_args.size(), 10000 );
    if ( rv < 0 )
    {
      Logger::Error( "SERVER", "Error while polling the server!" );
      exit( rv );
    }

    for ( size_t i = 1; i < poll_args.size(); ++i )
    {
      if ( poll_args[ i ].revents )
      {
        Connection* conn = connections[ poll_args[ i ].fd ];
        connection_io( conn );

        if ( conn->state == Connection::END )
        {
          connections[ conn->fd ] = nullptr;
          close( conn->fd );
        }
      }
    }

    if ( poll_args[ 0 ].revents )
    {
      accept_new_connection( connections, socket_connection_fd );
    }
  }
}