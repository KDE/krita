#ifndef QUIRK_NETINET_IN_H
#define QUIRK_NETINET_IN_H

/*
 * this is used for htonl mostly.
 *
 * while POSIX specifies <arpa/inet.h> as the portable
 * include file, <netinet/in.h> is used throughout
 * calligra.
 *
 */

#pragma message("winquirk: no netinet/in.h!")
#include <Winsock2.h>

#endif
