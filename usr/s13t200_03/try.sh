#! /bin/sh

sh ../../sys/Make2015.sh PokerOpe.c
../../sys/DeckInit.exe 10000 > Deck_03.txt
./PokerExec.exe s13t232 10000 Deck_03.txt 0
