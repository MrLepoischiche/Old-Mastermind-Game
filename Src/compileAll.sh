#!/bin/bash

gcc -o ../Server/MMSrvTCP MMSrvTCP.c -lncurses
gcc -o ../Server/MMSrvCod MMSrvCod.c -lncurses
gcc -o ../Server/MMSrvDec MMSrvDec.c -lncurses
gcc -o ../Client/MMCltTCP MMCltTCP.c -lncurses
gcc -o ../Client/MMCltCod MMCltCod.c -lncurses
gcc -o ../Client/MMCltDec MMCltDec.c -lncurses
