#! /bin/sh

sh ../../sys/BinCompile.sh ../../src 7 5 PokerOpe.c
../../sys/DeckInit.exe 10000 > Deck_03.txt
./PokerExec.exe s13t232 10000 Deck_03.txt 0
