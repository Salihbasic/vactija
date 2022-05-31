# Vactija
Bismillah. 

This is an unofficial Linux front-end for [Vaktija.ba](https://vaktija.ba/). It relies on the data provided by this service and presents it on the command-line interface.

# Dependences
Vactija uses [libcurl](https://github.com/curl/curl) to access API data and therefore also requires a working internet connection (at least once per day).

To parse JSON data, it depends on [jsmn](https://github.com/zserge/jsmn) which is a header library included in the program's source code.

# Installation and configuration
You will need `GNU Make` and `GNU C Compiler` (although other compilers might work) as well as `libcurl` to install Vactija:


1. Run `git clone https://github.com/Salihbasic/vactija`
2. `cd vactija`
3. `cp config.def.h config.h` (and edit the config according to your preferences)
4. Edit `Makefile` to change install directory, default is `/usr/local/bin/` (OPTIONAL)
5. Run `sudo make install clean` to install vactija

You might also wish to use `make release` or `make testrel` which will create a new directory within the `vactija` directory (either `release` or `testrel`). The testing release can be passed to GDB for debugging. Regular release is the binary like the one used by `install` except it is not installed to any directory.

If you wish to reconfigure the application, simply edit `config.h` and run `sudo make install clean` (or any other appropriate option) again.