#include "read.h"

int read_a( int fd, char* buf, size_t n )
{
  ssize_t read_result;

  while ( n > 0 )
  {
    read_result = read( fd, buf, n );
    if ( read_result <= 0 )
    {
      return -1;
    }

    n -= read_result;
    buf += read_result;
  }

  return 0;
}
