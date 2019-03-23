.PHONY: all clean

all: lisp

lisp: Sexp.o RunLISP.o main.c
	gcc -o lisp main.c RunLISP.o Sexp.o

RunLISP.o: RunLISP.c RunLISP.h
	gcc -c RunLISP.c

Sexp.o: Sexp.c Sexp.h
	gcc -c Sexp.c

clean:
	rm -f *.o
	rm -f lisp