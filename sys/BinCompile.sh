#! /bin/sh

/usr/bin/gcc -I${1} -DCHNG=${2} -DTAKE=${3} -o PokerExec PokerOpe.c ../../src/CardLib.c ../../src/PokerExec.c
