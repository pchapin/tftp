//**************************************************************************
// FILE   : Server.java
// SUBJECT: TFTP server program in Java
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

import java.io.IOException;

/**
 * Main class of a TFTP server. See RFC 1350.
 */
public class Server {

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
        
        System.out.println("Hello, World!");
    }

}
