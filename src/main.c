/*
Pseudo test program to get library working
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int
main(int argc, const char *argv[])
{
    int fd;
    void *buf;
    ssize_t size_read;
    int test_integer;

    if ( (fd = open(argv[1], O_RDONLY)) < 0)
    {
        // TODO error
        return -1;
    }

    size_read = read(fd, buf, 256);

    printf("Enter an integer: ");
    scanf("%d", &test_integer);  
    printf("Number = %d\n",test_integer);

    close(fd);
    
    return EXIT_SUCCESS;
}