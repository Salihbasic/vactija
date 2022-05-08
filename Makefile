CC = gcc

libs = -lcurl
relobj = vactija-cli.o vactija.o temporal.o jsmnutil.o cachefile.o jsmn.o
testobj = test.o vactija.o temporal.o jsmnutil.o cachefile.o jsmn.o

release : $(relobj)
	$(CC) -g -o vactija $(relobj) $(libs)

test : $(testobj)
	$(CC) -g -o vactija-test $(testobj) $(libs)

test.o : test/test.c test/test.h vactija.h util/jsmnutil.h util/temporal.h util/cachefile.h
	$(CC) -g -c test/test.c

vactija-cli.o : vactija-cli.c vactija.h config.h util/cachefile.h
	$(CC) -g -c vactija-cli.c

vactija.o : vactija.c vactija.h util/jsmnutil.h jsmn/jsmn.h util/temporal.h
	$(CC) -g -c vactija.c

jsmnutil.o : util/jsmnutil.c util/jsmnutil.h jsmn/jsmn.h
	$(CC) -g -c util/jsmnutil.c

temporal.o : util/temporal.c util/temporal.h
	$(CC) -g -c util/temporal.c

cachefile.o : util/cachefile.c util/cachefile.h
	$(CC) -g -c util/cachefile.c

jsmn.o : jsmn/jsmn.c jsmn/jsmn.h
	$(CC) -g -c jsmn/jsmn.c

.PHONY: clean
clean :
	rm -f *.o *-test