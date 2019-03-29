#!/usr/bin/zsh

echo -n > manyredefs.le

for i in {a..z}{a..z}; do
    echo "(define same '$i)" >> manyredefs.le
done

echo \
    "(load manyredefs)" \
    "\nsame" \
| ./lisp

rm manyredefs.le