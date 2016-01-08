#! /bin/sh

ins_num=01
dck_num=01

##--  get last dir name like s13t200_00
result_pwd=`pwd`
dir_name=`basename ${result_pwd}`_${ins_num}

./PokerExec ${dir_name} 1 Deck_${ins_num}.ini 1
