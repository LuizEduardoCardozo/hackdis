//
// Created by Fluency Academy on 17/03/23.
//

#include "write.h"

int write_a( int fd, const char* buf, size_t n )
{
  ssize_t write_result;

  while ( n > 0 )
  {
    write_result = write( fd, buf, n );
    if ( write_result <= 0 )
    {
      return -1;
    }

    n -= static_cast<size_t>( write_result );
    buf += write_result;
  }

  return 0;
}
