/*!
 * \file send_file.c
 * \author Peter C. Chapin
 * \brief Function for server side file transfers.
 *
 */

 #include "server.h"

//! Send a file to the client.
/*!
 * \return 0 if the transfer is successful; -1 otherwise.
 */
int send_file( int socket_handle, struct sockaddr_in6 *client_address, const char *file_name )
{
    return 0;
}
