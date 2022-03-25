-all: cal-compile new-cal-compile
cal-compile:
	gcc -c cal.c triples.c quadratic.c sum.c
	gcc cal.o sum.o quadratic.o triples.o -lm -o cal.exe

new-cal-compile:
	gcc -c cal-new.c admin.c triples.c quadratic.c sum.c
	gcc cal-new.o admin.o sum.o quadratic.o triples.o -lm -o cal-new.exe