#ifndef IOHOOKS_H
#define IOHOOKS_H

// Do I even need this?
int
real_open (const char *pathname, int flags, ...);
int
real_close (int fd);

#endif
