/*!
 * \file server.h
 * \author Peter C. Chapin
 * \brief Application header containing various global declarations.
 *
 */

#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <netinet/in.h>

int send_file( int socket_handle, struct sockaddr_in6 *client_address, const char *file_name );

#endif // SERVER_H_INCLUDED
