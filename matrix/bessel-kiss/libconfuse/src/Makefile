CFLAGS = -I. -Wall -DHAVE_CONFIG_H -DHAVE_STRCASECMP -fPIE

libconfuse.a: lexer.o confuse.o
	ar rc libconfuse.a confuse.o lexer.o

confuse.o: confuse.c config.h
	gcc $(CFLAGS) -c confuse.c

lexer.o: lexer.c config.h
	gcc $(CFLAGS) -c lexer.c

lexer.c: lexer.l
	flex -olexer.c -Pcfg_yy lexer.l

clean:
	rm libconfuse.a *.o

