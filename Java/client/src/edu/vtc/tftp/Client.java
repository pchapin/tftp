//**************************************************************************
// FILE   : Client.java
// SUBJECT: TFTP client program in Java
//
// Please send comments or bug reports to
//
//      Peter C. Chapin
//      Computer Information Systems
//      Vermont Technical College
//      Randolph Center, VT 05061
//      PChapin@vtc.vsc.edu
//***************************************************************************
package edu.vtc.tftp;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;

/**
 * Main class of a TFTP client. See RFC 1350.
 */
public class Client {

    public static void main(String[] args)
    {
        try {
            wrappedMain(args);
        }
        catch (Exception ex) {
            System.out.println("Unhandled exception: " + ex.getMessage());
        }
    }


    private static void wrappedMain(String[] args) throws IOException
    {
        // Get stuff from the command line if it is there.
        int port = 69;
        if (args.length >= 2) {
            port = Integer.parseInt(args[1]);
        }
        String host = "localhost";
        if (args.length >= 1) {
            host = args[0];
        }

        InetSocketAddress serverAddress = new InetSocketAddress(host, port);
        mainLoop(serverAddress);
    }


    private static void mainLoop(SocketAddress serverAddress) throws IOException
    {
        BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in, "US-ASCII"));

        System.out.println("Enter \"!quit\" to end.");
        while (true) {
            System.out.print("get> ");
            String line = consoleReader.readLine();

            if (line.equals("!quit")) break;
            DatagramSocket clientSocket = new DatagramSocket();
            FileReceiver receiver = new FileReceiver(line, clientSocket, serverAddress);
            receiver.doReceive();
        }
    }

}
