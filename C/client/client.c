/*!
 * \file client.c
 * \author Peter C. Chapin
 * \brief Trivial FTP client
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#ifndef S_SPLIT_S     // Workaround for splint.
#include <unistd.h>
#endif

#include "client.h"


//! This is the main loop of the program.
/*!
 * Get file names from the user and fetch the requested files from the server.
 *
 * \param server_address The IP/port address of the server host.
 */
static void main_loop(const struct sockaddr_in6 *server_address)
{
    int   socket_handle;
    char  file_name[128+2];
    char *end_ptr;

    // Keep getting file names from the user.
    printf("Enter \"!quit\" to end.\n");
    while (1) {

        printf("get> ");
        fgets(file_name, 128+2, stdin);
        if ((end_ptr = strchr(file_name, '\n')) != NULL) *end_ptr = '\0';

        // If the user has had enough, stop. Otherwise try to get the file desired. A future
        // version of this program may support additional commands.
        //
        if (strcmp(file_name, "!quit") == 0) break;

        // Create an appropriate socket. If it works, get the file.
        if ((socket_handle = socket(PF_INET6, SOCK_DGRAM, 0)) == -1) {
            perror("Unable to create socket");
        }
        else {
            receive_file(file_name, socket_handle, server_address);
            close(socket_handle);
        }
    }
}


// ============
// Main Program
// ============

int main(int argc, char **argv)
{
    struct addrinfo  getaddr_hints;
    struct addrinfo *lookup_result;
    struct sockaddr_in6 server_address;
    unsigned short    port = 69;

    // Do I have a command line argument? I need at least the server name.
    if (argc < 2) {
        fprintf(stderr, "Usage: %s server-name [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Do I have an explicit port number?
    if (argc == 3) {
        port = atoi(argv[2]);
    }

    // Look up the IP address associated with the desired host.
    memset( &getaddr_hints, 0, sizeof(struct addrinfo) );
    getaddr_hints.ai_family = AF_INET6;
    getaddr_hints.ai_flags = AI_V4MAPPED;
    if( getaddrinfo( argv[1], NULL, &getaddr_hints, &lookup_result ) != 0 ) {
        printf("Can't find IP address for host name: %s!", argv[1] );
        return EXIT_FAILURE;
    }
    server_address = *(struct sockaddr_in6 *)lookup_result->ai_addr;
    server_address.sin6_port = htons( port );
    // TODO: Echo back the IP and port addresses so the user can confirm their sensibility.

    // The main body of the program is here.
    main_loop(&server_address);

    return EXIT_SUCCESS;
}
