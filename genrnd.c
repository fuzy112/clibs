#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    time_t t;
    int count;

    time(&t);
    srand(t);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s count\n", argv[0]);
        return 1;
    }

    count = atoi(argv[1]);
    while (count-- > 0) {
        printf("%d\n", rand());
    }

    return 0;
}
