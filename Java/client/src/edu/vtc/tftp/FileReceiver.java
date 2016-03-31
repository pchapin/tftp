//**************************************************************************
// FILE   : FileReceiver.java
// SUBJECT: Class that knows how to receive a file using TFTP.
//
//***************************************************************************
package edu.vtc.tftp;

import java.net.DatagramSocket;
import java.net.SocketAddress;

public class FileReceiver {
    
    private String         fileName;
    private DatagramSocket clientSocket;
    private SocketAddress  serverAddress;

    /**
     * Constructs a FileReceiver. An instance of this class is responsible for receiving and storing a single file from
     * the TFTP server.
     *
     * @param fileName      The name of the file on the server host to receive. The file is stored locally in the
     *                      current folder regardless of any path that might be on the file name.
     * @param clientSocket  An initialized socket object on the client side to use for the transfer.
     * @param serverAddress The address (IP and port) of the listening server.
     */
    FileReceiver(String fileName, DatagramSocket clientSocket, SocketAddress serverAddress)
    {
        this.fileName      = fileName;
        this.clientSocket  = clientSocket;
        this.serverAddress = serverAddress;
    }


    /**
     * Receives the file using the parameters previously passed to the constructor.
     */
    void doReceive()
    {
        // Fill in the request packet...

        // Send the request...

        // Now go into a loop to retrieve the data blocks...
        // TODO: Time out receive operations.
        //
        while (true) {

            // Receive a packet from the server...

            // Make sure the receive was successful and the packet is a data packet...

            // Strip paths off file name. Be sure the output file is open...

            // If there was data in this packet and it's a new block...

            // Send acknowledgements for the "current" block or for duplicates of the previous
            // block. Send the ACKs to the "incoming address" instead of the server address. This
            // is because the server might use a different port for the data transfer...

            // Provide user feedback...

            // If this was the final packet, I am done. Indicate success...
        }

        
    }

}
