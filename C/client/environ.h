/*! \file    environ.h
    \brief   Defines the compilation and the target environments.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

This file contains settings that define the environment in which the program was compiled and
the environment where it runs. This file should be included into source files that need to
distinguish one environment from another. Conditional compliation directives can then be used to
select appropriate code for each environment.

I find it easier to use the symbols defined in this file than it is to use the symbols defined
by the various individual compilers. The symbols here are more consistent and more natural.
However, it is my intention for this file to use the compiler specific symbols to automatically
set appropriate values for the symbols defined here whenever possible.

Each symbol defined here is prefixed with 'e' (for "environment"). This is done because several
of the symbol names are fairly generic and they tend to collide with similarly named symbols in
other libraries.
*/

#ifndef ENVIRON_H
#define ENVIRON_H

//-----------------------------
//           Compiler
//-----------------------------

// The following are the allowed values of eCOMPILER.
#define eVANILLA     1  // Generic, Standard C++ only
#define eBORLAND     2  // Borland C++
#define eCOMPAQ      3  // Compaq C++
#define eGCC         4  // gcc
#define eIBM         5  // IBM's Visual Age C++
#define eMETROWERKS  6  // Metrowerks CodeWarrior
#define eMICROSOFT   7  // Microsoft Visual C++
#define eOPENWATCOM  8  // Open Watcom

// Choose your compiler! This file can autodetect all of the compilers mentioned above. If the
// compiler can't be autodetected it will default to eVANILLA.

#if defined(__BORLANDC__)
#define eCOMPILER eBORLAND
#endif

#if defined(__DECCXX)
#define eCOMPILER eCOMPAQ
#endif

#if defined(__GNUC__)
#define eCOMPILER eGCC
#endif

#if defined(__IBMCPP__)
#define eCOMPILER eIBM
#endif

#if defined(__MWERKS__)
#define eCOMPILER eMETROWERKS
#endif

#if defined(_MSC_VER)
#define eCOMPILER eMICROSOFT
#endif

#if defined(__WATCOMC__)
#define eCOMPILER eOPENWATCOM
#endif

#if !defined(eCOMPILER)
#define eCOMPILER eVANILLA
#endif

// It might make sense to encode the compiler version also.

//-------------------------------------
//           Operating System
//-------------------------------------

// The following are the allowed values of eOPSYS.
#define eMAC     1  // MacOS.
#define eDOS     2  // DOS and its variations.
#define eNETWARE 3  // NetWare NLM. Assume v4.x or higher (NDS support).
#define eOS2     4  // OS/2 (32 bit only).
#define ePOSIX   5  // POSIX is intended to support all Unix flavors.
#define eVMS     6  // DEC's VMS operating system.
#define eWIN32   7  // WinNT/2000/XP/Vista/7 only. Win95/98/Me are obsolete.

// Choose your operating system! In most cases this file can autodetect the operating system
// from the compiler that is being used. If that is not the case, you will have to specify the
// operating system if it matters. There is no default.

// Borland supports MS-DOS and Win32 programming.
#if eCOMPILER == eBORLAND
#if defined(__MSDOS__)
#define eOPSYS eDOS
#elif defined(__WIN32__)
#define eOPSYS eWIN32
#endif
#endif

// Assume that Compaq C++ is on Unix. Is this assumption safe? (What of VMS?)
#if eCOMPILER == eCOMPAQ
#define eOPSYS ePOSIX
#endif

// Assume gcc is on Unix (or at least something Unix-like).
#if eCOMPILER == eGCC
#define eOPSYS ePOSIX
#endif

// Visual Age C++ supports OS/2 and Win32 programming.
#if eCOMPILER == eIBM
#if defined(__TOS_OS2__)
#define eOPSYS eOS2
#elif defined(__TOS_WIN__)
#define eOPSYS eWIN32
#endif
#endif

// CodeWarrior supports Mac and Win32 programming.
#if eCOMPILER == eMETROWERKS
#if defined(macintosh)
#define eOPSYS eMAC
#elif defined(__INTEL__)
#define eOPSYS eWIN32
#endif
#endif

// Visual C++ supports Win32 and Mac(?!) programming.
#if eCOMPILER == eMICROSOFT
#if defined(_WIN32)
#define eOPSYS eWIN32
#elif defined(_MAC)
#define eOPSYS eMAC
#endif
#endif

// Open Watcom supports a variety of different systems.
#if eCOMPILER == eOPENWATCOM
#if defined (__DOS__)
#define eOPSYS eDOS
#elif defined(__OS2__)
#define eOPSYS eOS2
#elif defined(__NT__)
#define eOPSYS eWIN32
#elif defined(__NETWARE__)
#define eOPSYS eNETWARE
#elif defined(__LINUX__)
#define eOPSYS ePOSIX
#endif
#endif

//---------------------------------------------
//           Graphical User Interface
//---------------------------------------------

// The following are the allowed values of eGUI.
#define eNONE   1   // Text mode application.
#define ePM     2   // The OS/2 graphical interface. This also implies WPS.
#define eWIN    3   // Windows NT or Windows 95/98. (16 bit Windows ignored).
#define eXWIN   4   // X Windows.

// Choose your GUI. This file does not currently autodetect any GUI. The default GUI is eNONE.
// Note that if the operating system is eMAC its native GUI can be assumed. That is not the case
// for the other operating systems since they support text mode applications as an option.

#if !defined(eGUI)
#define eGUI eNONE
#endif

// Do a few checks to make sure the GUI selection makes sense.

#if eGUI == eWIN && eOPSYS != eWIN32
#error Can not specify the Windows GUI without the Win32 operating system!
#endif

#if eGUI == ePM && eOPSYS != eOS2
#error Can not specify the PM GUI without the OS/2 operating system!
#endif

//-----------------------------------
//           Multithreaded
//-----------------------------------

// When writing a multithreaded program, there are additional issues that must be considered.
// The symbol eMULTITHREADED will be defined in all such cases. This file can auto-detect this
// feature in some but not all cases.
//
// Note that if eMULTITHREADED is defined when eOPSYS is ePOSIX, Posix threads are implied. If
// eOPSYS is eWIN32, Windows threads are implied.
//

#if eCOMPILER == eBORLAND && defined(__MT__)
#define eMULTITHREADED
#endif

#if eCOMPILER == eCOMPAQ && defined(_REENTRANT)
#define eMULTITHREADED
#endif

#if eCOMPILER == eIBM && defined(_MT)
#define eMULTITHREADED
#endif

#if eCOMPILER == eMICROSOFT && defined(_MT)
#define eMULTITHREADED
#endif

#if eCOMPILER == eOPENWATCOM && defined(_MT)
#define eMULTITHREADED
#endif

#endif
