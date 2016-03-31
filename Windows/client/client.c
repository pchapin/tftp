/*****************************************************************************
FILE    : client.c
SUBJECT : TFTP client.
AUTHOR  : (C) Copyright 2010 by Peter C. Chapin

This is a simple TFTP client that uses Winsock. It allows the user to interactively get files
(for which the user knows the name).

This version does not timeout the recvfrom() calls. It can mostly get away with this because the
server will resend the last data block if it doesn't get the ACK from this program. There are
three situations where the lack of a timeout here will cause a problem. All three of them are
somewhat rare:

 1. The initial RRQ packet sent by this program is lost on the network.
 2. The server crashes or becomes unreachable during a transfer.
 3. All server retries are lost and the server gives up.
 
A more robust version of this program would timeout recvfrom() but this program should work
fairly well without it, even in cases where data is lost on the network. Here I'm assuming that
the server implemenation is robust.

This version also does not worry much about duplicate packets. It is careful to only save new
packets (duplicates are not saved) but it ACKs all packets it receives regardless. In a vast
majority of cases the only duplicate packet you might see would be the previous one. Such
packets would be sent as a result of a server retry after a lost ACK. In that case, ACKing the
packet is the correct (and necessary) thing to do. However, the server will not send a new
packet until it has gotten a positive acknowledgement for the current one. Thus the server
should never resend arbitrarly old packets. Such packets *might* wander on the network and
arrive here at the client long after they were sent. In that case the client should not ACK them
(they must have already been resent by the server and ACKed by the client if they are that old),
although doing so shouldn't cause an error.

To Do:

 + Allow the program to accept the filename(s) on the command line so that it is more easily
   scriptable.
   
 + Support optional numeric IP addresses in place of host names.
 
 + Implement some more interactive commands (such as a command that allows the user to change
   hosts and/or ports). This will require changing the prompt and introducing a formal "get"
   command as well.
   
 + Deal with timeouts while waiting for packets from the server.
 
 + This version of the program won't allow transfers of files greater than 32 MBytes because of
   the way block numbers wrap around after that. This should be fixed (how do servers handle the
   wrap-around?). Currently such large files are SILENTLY truncated. Bad. This program thinks
   the low numbered blocks are duplicates of previously seen blocks. It thus does not write them
   to disk.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

//
// This function returns a pointer to the basename part of the given Unix-style path. I use it
// so that files in a subdirectory on the server are placed in the current directory on the
// client. Other policies could be imagined.
//
static const char *convert_path(const char *path)
{
  const char *result = strrchr(path, '/');
  if (result == NULL) return path;
  return result + 1;
}


//
// This function does the actual grunt work of getting a file from the server. Notice that this
// function opens the output file inside the loop where data blocks are received from the
// server. This "just in time" approach means that I don't open the file (possibily overwriting
// an existing file) until I know for sure that I'm going to receive something from the server
// for it.
//
// This function also aborts if it encounters a problem that it can't figure out how to handle.
// In those cases the output file is probably corrupt (but not necessarily depending on when,
// exactly, the error occured). Often the caller can simply try again. This function returns
// zero for success and -1 if any kind of error occurs.
//
static int get_file(
  const char               *file_name,
        SOCKET              socket_handle,
  const struct sockaddr_in *server_address)
{
  // Allocate some memory.
  const int REQUEST_LENGTH = (int)(2 + strlen(file_name) + 1 + 5 + 1);
  const int DATA_LENGTH    = 512 + 4;
  const int ACK_LENGTH     = 4;
  char  buffer[516];

  // Various other data objects needed.
  struct sockaddr_in incoming_address; // Source address of incoming packet.
  int    address_size;                 // Size of incoming address structure.
  FILE  *output = NULL;                // Refers to the output file.
  int    send_count;                   // Number of bytes actually sent.
  int    recv_count;                   // Number of bytes actually received.
  int    op_code;                      // Operation code in incoming packet.
  unsigned short block_number;         // Block number in incoming packet.
  long   block_count = 0;              // The total number of blocks received.
  long   byte_count = 0;               // The total number of data bytes received.
  int    return_code = -1;             // Assume we have an error unless proven otherwise.
    
  // Fill in the request packet.
  buffer[0] = 0;     // RRQ (16 bits in big endian order).
  buffer[1] = 1;
  strcpy(&buffer[2], file_name);
  strcpy(&buffer[2 + strlen(file_name) + 1], "octet");
  
  // Send the request.
  send_count = sendto(socket_handle, buffer, REQUEST_LENGTH, 0,
    (const struct sockaddr *)server_address, sizeof(struct sockaddr_in));
  if (send_count == SOCKET_ERROR || send_count != REQUEST_LENGTH) {
    printf("\nError sending read request! Error code = %d\n", WSAGetLastError());
    return return_code;
  }
  
  // Now go into a loop to retrieve the data blocks. Currently there is no facility for timeouts
  // on the receive operation. There should be.
  //
  while (1) {
  
    // Receive a packet from the server.
    address_size = sizeof(struct sockaddr_in);
    recv_count = recvfrom(socket_handle, buffer, DATA_LENGTH, 0,
      (struct sockaddr *)&incoming_address, &address_size);
    
    // Make sure the receive operation was happy.  
    if (recv_count == SOCKET_ERROR) {
      printf("\nError receiving data! Local file may be truncated. Error code = %d\n",
        WSAGetLastError());
      break;
    }
    
    // Make sure the received packet is a data packet.
    op_code = buffer[1];
    if (op_code == 5) {
      printf("\nError! Message from remote host: %s\n", &buffer[4]);
      break;
    }
    if (op_code != 3) {
      printf("\nUnexpected packet received! Type = %d\n", op_code);
      break;
    }
    
    // Be sure the output file is open. Strip paths off the name.
    if (!output) {
      if ((output = fopen(convert_path(file_name), "wb")) == NULL) {
        printf("\nUnable to open local file %s for output.\n", file_name);
        break;
      }
    }

    // What block did I just receive?
    block_number = ((unsigned short)buffer[2] << 8) | (unsigned short)buffer[3];

    // If there was data in this packet and it's a new block...
    if ((recv_count - 4 > 0)  &&  (block_number > block_count)) {
      byte_count += recv_count - 4;
      block_count++;
      fwrite(&buffer[4], 1, recv_count - 4, output);
      if (ferror(output)) {
        printf("\nError writing data! Disk full? Local file may be truncated.\n");
        break;
      }
    }
    
    // Send acknowledgements for the "current" block or for duplicates of the previous block.
    // Notice that I send the ACKs to the "incoming address" instead of the server address. This
    // is because the server might use a different port for the data transactions.
    //
    buffer[0] = 0;   // ACK
    buffer[1] = 4;
    // Acknowledge the latest data packet.

    send_count = sendto(socket_handle, buffer, ACK_LENGTH, 0,
      (const struct sockaddr *)&incoming_address, address_size);
    if (send_count == SOCKET_ERROR || send_count != ACK_LENGTH) {
      printf("\nError sending acknowledgement! Local file may be truncated. Error code = %d\n",
        WSAGetLastError());
      break;
    }
    
    printf("\r%ld bytes (%ld blocks) received", byte_count, block_count);
    fflush(stdout);
    
    // If this was the final packet, I am done. Indicate success.
    if (recv_count - 4 != 512) { return_code = 0; break; }
  }
  
  
  // Clean up.
  if (output) {
    if (fclose(output) == EOF) {
      printf("\nError closing output! Disk full? Local file may be truncated.\n");
      
      // Here I put return_code back to -1 since I'm not as successful as I thought. 
      return_code = -1;
    }
  }  
  printf("\n");
  return return_code;
}


//
// This is the main loop of the program. It is here were I get file names from the user and
// fetch the requested files from the server. Note that I create a separate socket for each file
// transaction. If I don't do this the second file transaction looks like it's part of the same
// pseudo- connection as the first (since the source port the server sees on the UDP packets is
// the same). This might cause some confusion particularly if the first transaction fails for
// some reason.
//
static void main_loop(const char *host_name, unsigned short port)
{
  SOCKET              socket_handle;
  struct hostent     *host_information;
  struct sockaddr_in  server_address;

  char  file_name[128+2];
  char *end_ptr;
  
  // First fill in the server address structure. I can't do anything without this so I might as
  // well take care of it now. This won't handle numeric addresses but I should do so at some
  // point. It might also be nice to eventually allow an interactive command that changes hosts.
  //
  host_information = gethostbyname(host_name);
  if (host_information == NULL) {
    printf("Can't find IP address for host name: %s! Error code = %d\n",
      host_name, WSAGetLastError());
    return;
  }
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port   = htons(port);
  memcpy(&server_address.sin_addr.S_un.S_addr, host_information->h_addr_list[0], 4);
  
  // Keep getting file names from the user.
  printf("Enter \"!quit\" to end.\n");
  while (1) {
  
    printf("get> ");
    fgets(file_name, 128+2, stdin);
    if ((end_ptr = strchr(file_name, '\n')) != NULL) *end_ptr = '\0';
    
    // If the user has had enough, stop. Otherwise try to get the file
    // desired. A future version of this program may support additional
    // commands.
    //
    if (strcmp(file_name, "!quit") == 0) break;
    
    // Create an appropriate socket. If it works, get the file.
    if ((socket_handle = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
      printf("Can't create socket! Error code = %d\n", WSAGetLastError());
    }
    else {
      get_file(file_name, socket_handle, &server_address);
      closesocket(socket_handle);
    }
  }
}


// ============
// Main Program
// ============

int main(int argc, char **argv)
{
  WSADATA        winsock_information;
  unsigned short port = 69;

  // Do I have a command line argument? I need at least the server name.
  if (argc < 2) {
    fprintf(stderr, "Usage: %s server-name [port]\n", argv[0]);
    return 1;
  }

  // Do I have an explicit port number?
  if (argc == 3) {
    port = atoi(argv[2]);
  }

  // Try to initialize Winsock.
  if (WSAStartup(MAKEWORD(2, 0), &winsock_information) != 0) {
    fprintf(stderr, "Can't find a suitable Winsock\n");
    return 1;
  }
  if (!(LOBYTE(winsock_information.wVersion) == 2 &&
        HIBYTE(winsock_information.wVersion) == 0)) {
    fprintf(stderr, "Can't find a suitable Winsock\n");
    WSACleanup();
    return 1;
  }

  // The main body of the program is here.
  main_loop(argv[1], port);

  WSACleanup();
  return 0;
}
