.PHONY: all clean

all: lisp

lisp: Sexp.o RunLISP.c
	gcc -o lisp RunLISP.c Sexp.o

Sexp.o: Sexp.c Sexp.h
	gcc -c Sexp.c

clean:
	rm -f *.o
	rm -f lisp