
Trivial FTP
===========

This folder contains Trivial FTP (TFTP) client/server programs in both C and Java. See RFC-1350
for more information about the TFTP protocol. These programs are used in Vermont Technical
College's Network Programming course as a vehicle for discussing various network programming
related issues. It is the intention for these programs to be robust and sufficiently complete to
be useful for actual application.

The C programs consist of two Code::Blocks projects and are compiled with clang v3.1. There is a
single Code::Blocks workspace file that loads both projects at once. The Java programs consist
of an IntelliJ IDEA project with two modules and are compiled with Java 7.

The C programs use Doxygen for internal documentation. The Java programs use the standard
JavaDoc tool. The C programs use CUnit for unit testing. The Java programs use JUnit. The
functionality of the C and Java programs are intended to be comparable but some differences
exist due to the different features provided by the two environments.

The ROOT folder contains a few files that can be served with, for example, the standard TFTP
server. This is useful for testing; the clients here can be exercised against the standard
server (and also the standard clients can be exercised against the servers here).

The programs described above are all written for the Unix platform. However, this code base also
includes client/server programs in C for Windows. A Visual Studio solution file and an Open
Watcom project file are provided at the root; load these files into their corresponding
environments to build the Windows programs.

Peter C. Chapin  
PChapin@vtc.vsc.edu
