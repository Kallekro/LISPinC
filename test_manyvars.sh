#!/usr/bin/zsh

echo -n > manyvars.le

for i in {a..z}{a..z}; do
    echo "(define $i '$i)" >> manyvars.le
done

echo \
    "(load manyvars)" \
    "\naa" \
    "\nac" \
    "\nbb" \
    "\nze" \
    "\nzz" \
| ./lisp

rm manyvars.le