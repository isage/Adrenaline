# Bits

Bits of code meant to be shared by multiple sub-projects and doesn't make much sense to export as PSP library for reuse (e.g. Only one ISO driver is run at a time so it doesn't make sense to export from `systemctrl` as doing so will increase the size of the "systemctrl.prx" and the memory usage even when no iso driver is currently being used).

 - `iso_common.c`: Common code related to PSP ISO formats
 - `ttystdio.c`: Common code to patch PSP stdio to output in the VITA tty (mostly for debug print) used by SystemControl and Pentazemin