# Vactija
Bismillah. 

This is an unofficial Linux front-end for [Vaktija.ba](https://vaktija.ba/). It relies on the data provided by this service and presents it on the command-line interface.

# Dependences
Vactija uses [libcurl](https://github.com/curl/curl) to access API data and therefore also requires a working internet connection (at least once per day).

To parse JSON data, it depends on [jsmn](https://github.com/zserge/jsmn) which is a header library included in the program's source code.

# Installation and configuration
You will need `GNU Make` and `GNU C Compiler` (although other compilers might work) as well as `libcurl` to install Vactija:

```
git clone https://github.com/Salihbasic/vactija
cd vactija

cp config.def.h config.h
-- (now edit config.h according to your needs)

make release clean
```
The built binary can be found in the newly created `release/` directory.

If you wish to reconfigure the application, simply editing the `config.h` file and running `make release clean` again will suffice.

# Licence
Excepting dependency code and API data which fall under their respective copyright owners' licences, all Vactija code written by me is hereby dedicated to the public domain for free and unrestricted use, without any warranties or liability for its use by other entities.