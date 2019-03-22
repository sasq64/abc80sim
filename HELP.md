Usage: ./abc80 [options] [ihex_files...]
Simulate a microcomputer from the Luxor ABC series.


Options:
       --abc80             simulate an ABC80 (default)
       --abc802            simulate an ABC802
  -4,  --40                start in 40-column mode
  -8,  --80                start in 80-column mode (default)
  -b,  --no-basic          no BASIC ROM (uninitialized RAM instead)
       --basic             reverts the --no-basic option
  -d,  --no-device         no device driver ROMs
       --device            reverts the --no-device option
  -t,  --trace event,...   trace various events (see "--trace help")
  -Ft, --tracefile file    redirect trace output to a file
  -v, - -version           print the version string
  -h,  --help              print this help message
  -s,  --speed #.#|max     set the CPU frequency to #.# MHz (default 3.0)
       --color             allow ABC800C-style color (default)
       --no-color          black and white only
  -Dd, --diskdir dir       set directory for disk images (default abcdisk)
  -Df, --filedir dir       set directory for file sharing (default abcdir)
  -Ds, --scrndir dir       set directory for screen shots (default .)
  -Dd, --dumpdir dir       set directory for memory dumps (default .)
  -Cp, --printcmd cmd      set command to launch a print job (* = filename)
  -Fc, --casfile file      input file for cassette (CAS:)
  -Lc, --caslist file      read list of files for the cassette from a file
  -e,  --console           enable console output device (PRC:)
  -Fe, --consolefile file  enable console output device to a file
       --detach            detach from console if run from a command line

Options for ABC80 only:
  -k, --kb #              set the memory size K (1-32 or 64)
      --old-basic         run BASIC 1.0 (checksum 11273)
      --11273             same as --old-basic
      --new-basic         run BASIC 1.2 (checksum 9913)
      --9913              same as --new-basic
      --faketype          fake short keystrokes (default > 12.5 MHz)
      --realtype          true key up/down emulation (default < 12.5 MHz)

Options for ABC802 only:
 -Fm, --memfile file      load a file into the ABC802 MEM: device

The simulator supports the following hotkeys:
  Alt-q    quit the simulator
  Alt-s    take a screenshot
  Alt-r    CPU reset
  Alt-n    send NMI
  Alt-m    dump memory as currently seen from the CPU
  Alt-u    dump underlying RAM only (even nonexistent)
  Alt-f    turn faketype on or off
