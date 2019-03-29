#!/usr/bin/zsh

echo \
    "(define x 'abc)" \
    "\nx" \
    "\n(define y 'cba)" \
    "\n(cons x y)" \
    "\nx" \
    "\n(define x y)" \
    "\nx" \
    "\ny" \
    "\n(load listfunctions)" \
    "\n(append '(a b c) '(d e f))" \
    "\n(car (cdr (append '(a b c) '(d e f))))" \
    "\n(define lo (cdr (cddr '(h e l l o))))" \
    "\nlo" \
    "\n(save testout)" \
| ./lisp