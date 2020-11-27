/*
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <zlib.h>

int version[3] = {0, 0, 0};

static void decode(char *str)
{
    int n;
    for (n = 0; n < 3 && str; n++) {
        char *pnt = strchr(str, '.');
        if (pnt) {
            *pnt++ = '\0';
        }
        version[n] = atoi(str);
        str = pnt;
    }
}

int main(void)
{
    decode(strdup(zlibVersion()));
    return
        (version[0] < 1 ||
         (version[0] == 1 &&
          (version[1] < 1 ||
           (version[1] == 1 && version[2] < 4))));
}
