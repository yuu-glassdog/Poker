#! /bin/sh

sh ../../sys/Make2015.sh PokerOpe.c
../../sys/DeckInit.exe 1000 > Deck_03.txt
./PokerExec.exe s13t232 1000 Deck_03.txt 0
