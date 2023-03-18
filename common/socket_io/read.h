#ifndef HACKDIS_READ_H
#define HACKDIS_READ_H

#include <cstdint>
#include <cstdio>
#include <unistd.h>

int read_a( int fd, char* buf, size_t n );

#endif
