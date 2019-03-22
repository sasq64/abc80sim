This version has been substantially modified by H. Peter Anvin
<hpa@zytor.com> from the original version.  The original README is
included but may not apply anymore.

Version 2.1 includes support for undocumented Z80 instructions, and
uses SDL for output.  It supports ABC80 display in 40- or 80-character
modes.

It also includes the PR:/PRA:/PRB: "special" printer device from the
ABC80-in-an-FPGA project from www.abc80.org instead of the magic
UNX:/LIB: device.  This allows abc80sim to be used as a development
platform.

I would like to move both display and printing to Qt rather than SDL
and lpr, but I have no idea when I will have time for that.

Files to be accessed with PRA: (text) or PRB: (binary) should live in
the "abcdir" subdirectory; the "abcdisk" subdirectory can contain disk
images in UFD-DOS format.


	---------------------


This directory contains the source for an ABC80 emulator under X-windows. The
program is built around a Z80 emulator by David Gingold <gingold@think.com>
and Alec Wolman <wolman@crl.dec.com> who wrote it to use in an emulator for
TRS-80. I have corrected a few bugs in the emulator and also modified it
slightly to fit in the ABC80 model.

Beware that the program runs at maximum CPU at all times, it never
blocks for IO except when reading and writing files.

The program emulates most of the functionality in the ABC80 including
the real time clock. The sound chip, and most of the PIO functions
(like the casette-tape inteface) are not emulated though. There is
however a new device called UNX: which get installed as the default
device. This allows reading and writing of files to the UNIX
filesystem. The list of error code explanations is now emulated :-)

If the program crashes your X-server will probably be left with auto-repeat
turned off. This is fixed with the command: "xset r". It might be a good idea
to define a shell alias as "abc80; xset r".


Have fun!

--Jonas Yngvesson (jonas-y@isy.liu.se)


Building and installation
-------------------------

1. Edit the Makefile. You might need to change the compiler and its
   options. You need an ANSI-C compiler.
   You will probably also like to change the following macros: 
   ABCDIR, BINDIR, MANDIR and MANEXT.

   ABCDIR should point to a directory where some
   program specific files are stored, the PROM contents for example. The font
   used in the window are also stored in a directory in ABCDIR.

   BINDIR should point to the directory where the executable
   program should be installed. 

   MANDIR should point to the directory where the manual page should
   be installed with extension indicated by MANEXT

   The variable DEFINES should be used for any other configuration options,
   read the Makefile to see what options are available.

2. Type "make install". Hopefully that will be enough.


Acknowledgments
---------------
David Gingold <gingold@think.com>
Alec Wolman <wolman@crl.dec.com>
  Wrote the excellent z80-emulator on which the whole program is based.

Anders Andersson <andersa@Mizar.DoCS.UU.SE>
  Corrected several important bugs, contributed to the screen handling
  and generally provided a lot of good comments and suggestions.

Peter Johansson <d89-pjo@nada.kth.se>
  Corrected the file IO functions.

Niclas Wiberg <nicwi@isy.liu.se>
  Fixed the OpenWindows fonts. Provided several ideas and a lot
  of support in the beginning of the development.

Jan Danielsson <jan@isy.liu.se>
  Helped me read the PROMs in their DATA IO equipment.

Bo Kullmar <bk@kullmar.kullmar.se>
  Helped me get in touch with DIAB.

DIAB
  Gave me permission to distribute the PROM-contents.

Kjell Enblom <kjell-e@lysator.liu.se>
  Lent me his copy of "Avancerad programmering på ABC80"


Revision history
----------------
1.0:   First public (well) release.

1.1:   File IO handling fixed (Peter Johansson)
       Made sure the X-window was mapped before drawing in it.
       Added fonts for OpenWindows.

1.1.1: Fixed bug in font handling which caused SIGSEGV under OpenWindows.
       Fixed fonts so that DEL (chr$(127)) worked under OpenWindows.

1.1.2: Use od setitimer() instead of ualarm() for portability.
       Cleaned up code, added checks for failing malloc()s, removed
       private declarations of system functions, changed bzero() to memset().

1.1.3: Support in the Makefile for small endian machines.
       Support for systems without SIGIO.

1.2    Fix of frustrating bug which caused loss of typed characters on slow
       X-servers (read: OpenWindows).
       The UNX: device seem to be working properly at last.
       New device LIB: to read contents of current directory.
       The much asked for list of error explanations is now available as
       a window wich can be pulled out from under the "main" window. It
       doesn't behave quite right when bringing windows back and front in the
       stack yet, and the text is *very* small, but it's there.
       You don't exit the emulator bye hitting "F1" anymore (it wasn't very
       portable), instead, give the command "BYE" and you will exit to the
       (D)OS :-).
       The "device driver" is now stored in Intel-hex format and the file
       z80asm.tar contains an assembler and disassembler which can be used for
       hacking that and other things to be read into the emulator.
       There is a man-page.
       Various minor bugfixes and portability changes.
       A couple of stupid BASIC example files (*.BAC) is shipped in the
       realese. You might find LIB.BAC entertaining :-)
