/*!
 * \file client.h
 * \author Peter C. Chapin
 * \brief Application header containing various global declarations.
 *
 */

#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <arpa/inet.h>

int receive_file(
    const char *file_name,
          int   socket_handle,
    const struct sockaddr_in6 *server_address);

#endif // CLIENT_H_INCLUDED
