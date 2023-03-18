#ifndef HACKDIS_WRITE_H
#define HACKDIS_WRITE_H

#include <cstdint>
#include <cstdio>
#include <unistd.h>

int write_a( int fd, const char* buf, size_t n );

#endif
