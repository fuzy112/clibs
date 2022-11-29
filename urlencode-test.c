#include "urlencode.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char *url;
    char *encoded_url;

    if (argc < 2) {
        fprintf(stderr, "usage: %s URL\n", argv[0]);
        return 1;
    }

    url = argv[1];
    encoded_url = urlencode(url, URLENCODE_NO_RESV);

    if (!encoded_url) {
        perror(url);
        return 1;
    }

    puts(encoded_url);
    free(encoded_url);

    return 0;
}
