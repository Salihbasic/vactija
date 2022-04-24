libs = -lcurl
relobj = vactija-cli.o vactija.o temporal.o jsmnutil.o cachefile.o jsmn.o
testobj = test.o vactija.o temporal.o jsmnutil.o cachefile.o jsmn.o

release : $(relobj)
	gcc -o vactija $(relobj) $(libs)

test : $(testobj)
	gcc -g -o vactija-test $(testobj) $(libs)

test.o : test/test.c test/test.h vactija.h util/jsmnutil.h util/temporal.h util/cachefile.h
	gcc -c test/test.c

vactija-cli.o : vactija-cli.c vactija.h
	gcc -c vactija-cli.c

vactija.o : vactija.c vactija.h util/jsmnutil.h jsmn/jsmn.h util/temporal.h
	gcc -c vactija.c

jsmnutil.o : util/jsmnutil.c util/jsmnutil.h jsmn/jsmn.h
	gcc -c util/jsmnutil.c

temporal.o : util/temporal.c util/temporal.h
	gcc -c util/temporal.c

cachefile.o : util/cachefile.c util/cachefile.h
	gcc -c util/cachefile.c

jsmn.o : jsmn/jsmn.c jsmn/jsmn.h
	gcc -c jsmn/jsmn.c

.PHONY: clean
clean : 
	-rm vactija-test $(testobj)