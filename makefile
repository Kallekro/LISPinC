.PHONY: all clean

all: lisp

lisp: Sexp.o RunLISP.o main.c
	gcc -o lisp main.c RunLISP.o Sexp.o -g

RunLISP.o: Sexp.o RunLISP.c RunLISP.h
	gcc -c RunLISP.c -g

Sexp.o: Sexp.c Sexp.h
	gcc -c Sexp.c -g

clean:
	rm -f *.o
	rm -f lisp