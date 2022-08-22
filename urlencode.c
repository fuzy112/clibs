
#include "urlencode.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

__attribute_const__ static bool is_unreserved(int ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
           (ch >= '0' && ch <= '9') || (ch == '-') || (ch == '_') ||
           (ch == '.') || (ch == '~');
}

__attribute_const__ static bool is_reserved(int ch)
{
    return ch == '!' || ch == '#' || ch == '$' || ch == '&' || ch == '\'' ||
           ch == '(' || ch == ')' || ch == '*' || ch == '+' || ch == ',' ||
           ch == '/' || ch == ':' || ch == ';' || ch == '=' || ch == '?' ||
           ch == '@' || ch == '[' || ch == ']';
}

static const char xdigits[16] = {
    [0] = '0',   [1] = '1',   [2] = '2',   [3] = '3',
    [4] = '4',   [5] = '5',   [6] = '6',   [7] = '7',
    [8] = '8',   [9] = '9',   [0xA] = 'A', [0xB] = 'B',
    [0xC] = 'C', [0xD] = 'D', [0xE] = 'E', [0xF] = 'F',
};

char *urlencode(const char *url, int flags)
{
    char *encoded;
    const char *p;
    size_t i;
    size_t len = 0;

    for (p = url; *p != '\0'; ++p) {
        char ch = *p;

        if (is_unreserved(ch))
            len += 1;
        else
            len += 3;
    }

    encoded = malloc(len + 1);
    if (encoded == NULL)
        return NULL;

    for (p = url, i = 0; *p != '\0'; ++p) {
        int ch = *p;

        if (is_unreserved(ch)) {
            encoded[i++] = ch;
        } else if ((flags & URLENCODE_NO_RESV) && is_reserved(ch)) {
            encoded[i++] = ch;
        } else {
            encoded[i++] = '%';
            encoded[i++] = xdigits[(ch / 16)];
            encoded[i++] = xdigits[(ch % 16)];
        }
    }
    encoded[i] = '\0';

    return encoded;
}
