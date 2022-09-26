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
    int fd, fd2;
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

    if ( (fd2 = open("./test_data/data2", O_RDONLY)) < 0)
    {
        // TODO error
        printf("ERROR: couldn'T open file 2 in second test\nfd: %d\n", fd2);
        return -1;
    }

    close(fd);
    close(fd2);

    return EXIT_SUCCESS;
}
