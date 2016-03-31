/*!
 * \file server.c
 * \author Peter C. Chapin
 * \brief Trivial FTP client
 *
 * \todo Error messages should be logged rather than sent to the console.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#ifndef S_SPLIT_S     // Workaround for splint.
#include <unistd.h>
#endif

#include "server.h"

#define REQUEST_BUFFER_LENGTH 512

static char *extract_file_name( unsigned char *request_buffer )
{
    // TODO: Implement me!
    return NULL;
}


static void send_error_message( int socket_handle, struct sockaddr_in6 *client_address )
{
    char error_datagram[20 + 1];

    error_datagram[0] = 0x00;  // Opcode == 5.
    error_datagram[1] = 0x05;
    error_datagram[2] = 0x00;  // ErrorCode == 0 (not defined).
    error_datagram[3] = 0x00;
    strcpy( &error_datagram[4], "Not implemented!" );

    // Send it to the client. Don't worry about if the send succeeds for fails.
    sendto(
        socket_handle,   // The socket for client communications.
        error_datagram,  // Datagram to send.
        21,              // Length of the datagram.
        0,               // Flags (none selected).
        (struct sockaddr *)client_address,  // Destination address.
        sizeof(struct sockaddr_in6)         // Size of the distination address structure.
    );
}


// ============
// Main Program
// ============

int main( int argc, char **argv )
{
    int listen_handle;  // Socket for incoming client requests.
    int socket_handle;  // Handle for bulk client communication.

    struct sockaddr_in6 server_address;  // Listening address.
    struct sockaddr_in6 client_address;  // Address of client.
    socklen_t client_length;

    // Buffer to hold request message.
    unsigned char request_buffer[REQUEST_BUFFER_LENGTH];
    ssize_t request_count;

    unsigned short port = 69;  // Port number to listen on.
    pid_t child_id;            // Child process ID.
    const char *file_name;     // Name of file client wants to read.


    // Do I have an explicit port number?
    if( argc == 2 ) {
        port = atoi( argv[1] );
    }

    // Create the server socket.
    if( (listen_handle = socket( PF_INET6, SOCK_DGRAM, 0) ) == -1 ) {
        perror( "Unable to create socket" );
        return EXIT_FAILURE;
    }

    // Prepare the server socket address structure.
    memset( &server_address, 0, sizeof(server_address) );
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons( port) ;

    // Bind the server socket.
    if (bind(listen_handle, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror( "Unable to bind listening address" );
        close( listen_handle );
        return EXIT_FAILURE;
    }

    while( 1 ) {
        // Call recvfrom() to get a request datagram from the client.
        client_length = sizeof( client_address );
        request_count = recvfrom(
            listen_handle,          // Socket for receiving request.
            request_buffer,         // Pointer to buffer for request.
            REQUEST_BUFFER_LENGTH,  // Size of the request buffer.
            0,                      // Flags (none selected).
            (struct sockaddr *)&client_address,  // Pointer to structure for client address.
            &client_length                       // Pointer to variable holding size of address.
        );

        if( request_count == -1 ) {
            perror( "Error while receiving client request" );
        }
        // Otherwise try to create a child process for this transfer...
        else if( (child_id = fork( )) == -1 ) {
            perror( "Could not create child process for client" );
        }
        // Otherwise if we are the child...
        else if( child_id == 0 ) {
            close( listen_handle );

            // Create a fresh socket in the child to communicate with the client.
            if( (socket_handle = socket( PF_INET6, SOCK_DGRAM, 0) ) == -1 ) {
                perror( "Unable to create socket" );
                exit( EXIT_FAILURE );
            }

            // Extract the file name from the request.
            if( (file_name = extract_file_name( request_buffer )) == NULL ) {
                send_error_message( socket_handle, &client_address );
                close( socket_handle );
                exit( EXIT_SUCCESS );
            }

            // Send the file!
            send_file( socket_handle, &client_address, file_name );
            close( socket_handle );
            exit( EXIT_SUCCESS );
        }
    }

    return EXIT_SUCCESS;
}
