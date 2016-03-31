/*!
 * \file receive_file.c
 * \author Peter C. Chapin
 * \brief Function for client side file transfers.
 *
 */

#include <stdio.h>
#include <string.h>

#include "client.h"
#include "Timer.h"

//! Receive a file from the server.
/*!
 * \param file_name The name of the file to receive from the server.
 * \param socket_handle The UDP socket to use for communication with the server.
 * \param server_address Pointer to the server's address structure.
 *
 * \return 0 if the transfer is successful; -1 otherwise.
 */
int receive_file(
    const char *file_name,
          int   socket_handle,
    const struct sockaddr_in6 *server_address )
{
    // Allocate some memory.
    const int REQUEST_LENGTH = (int)( 2 + strlen(file_name) + 1 + 5 + 1 );
    const int DATA_LENGTH    = 512 + 4;
    const int ACK_LENGTH     = 4;
    char  buffer[516];

    // Various other data objects needed.
    struct      sockaddr_in6 incoming_address; // Source address of incoming packet.
    socklen_t   address_size;       // Size of incoming address structure.
    FILE       *output = NULL;      // Refers to the output file.
    const char *simple_file_name;   // The file name without the path.
    int         recv_count;         // Number of bytes actually received.
    int         op_code;            // Operation code in incoming packet.
    unsigned short block_number;    // Block number in incoming packet.
    long        block_count =  0;   // The total number of blocks received.
    long        byte_count  =  0;   // The total number of data bytes received.
    int         return_code = -1;   // Assume we have an error unless proven otherwise.

    // Used to time the transfer.
    Timer stopwatch;
    long  total_time;
    Timer_initialize( &stopwatch );

    // Fill in the request packet
    buffer[0] = 0;  // RRQ op-code.
    buffer[1] = 1;
    strcpy( &buffer[2], file_name );     // TODO: Make sure file_name is not too long.
    strcpy( &buffer[2 + strlen(file_name) + 1], "octet");

    Timer_start( &stopwatch );
    // Send the request.
    // TODO: Check return value.
    sendto(
        socket_handle,
        buffer,
        REQUEST_LENGTH,
        0,
        (const struct sockaddr *)server_address,
        sizeof(*server_address));

    // Now go into a loop to retrieve the data blocks.
    while( 1 ) {

        // Receive a packet from the server.
        // TODO: Time out receive operation.
        address_size = sizeof( incoming_address );
        recv_count = recvfrom(
            socket_handle,
            buffer,
            DATA_LENGTH,
            0,
            (struct sockaddr *)&incoming_address,
            &address_size );

        // Make sure the receive was successful.
        if( recv_count == -1 ) {
            perror( "recvfrom failed" );
            continue;  // Do we really want to do this?
        }

        // Make sure the received packet is a data packet.
        // TODO: Deal with unexpected packet types.
        // TODO: Verify that the error packet is really long enough.
        op_code = (buffer[0] << 8) | buffer[1];
        if( op_code == 5 ) {
            printf( "Error from server: %s\n", &buffer[4] );
            break;  // Do we really want to do this?
        }

        // Assume we have a DATA packet.
        block_number = (buffer[2] << 8) | (buffer[3] & 0x00FF);

        // Strip paths off file name. Be sure the output file is open.
        if( output == NULL ) {
            // TODO: This doesn't handle trailing slash characters very well but presumably they would cause errors anyway.
            simple_file_name = strrchr( file_name, '/' );
            if( simple_file_name == NULL )
                simple_file_name = file_name;
            else
                simple_file_name = simple_file_name + 1;

            output = fopen( simple_file_name, "w" );
            if( output == NULL ) {
                printf( "Unable to open %s (after receiving block #%u)", simple_file_name, block_number );
            }
        }

        // If there was data in this packet and it's a new block...
        // BUG: If the file contains more than 2^16 blocks this logic gets confused.
        if( (recv_count > 4) && (block_number == block_count + 1) ) {
            block_count++;
            byte_count += (recv_count - 4);
            fwrite( &buffer[4], 1, recv_count - 4, output );
       }

        // Send acknowledgements for the "current" block or for duplicates of the previous block.
        // TODO: Check return value.
        buffer[0] = 0;  // ACK op-code.
        buffer[1] = 4;
        sendto(
            socket_handle,
            buffer,
            ACK_LENGTH,
            0,
            (const struct sockaddr *)&incoming_address,
            sizeof(incoming_address));

        // Provide user feedback.
        printf( "\rReceived: %ld bytes", byte_count );
        fflush( stdout );


        // If this was the final packet, I am done. Indicate success.
        if( recv_count - 4 < 512 ) {
            return_code = 0;
            break;
        }
    }

    // Clean up (close output file if appropriate, etc).
    if( output != NULL ) {
        printf( "\n" );
        fclose( output );
    }
    Timer_stop( &stopwatch );
    total_time = Timer_time( &stopwatch );
    if( total_time > 1 ) {
        printf( "Transfer time: %ld.%03ld seconds; Transfer rate: %.3e bytes/s\n",
                total_time / 1000, total_time % 1000, ((double)byte_count * 1000) / total_time );
    }

    return return_code;
}
