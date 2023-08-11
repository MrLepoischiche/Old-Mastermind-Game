#include <fcntl.h>
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "mast_vars.h"

#define PLAT_COL 16
#define PLAT_PAIR 9

WINDOW *mainWin, *decWin, *invWin;
int fdReadDec;

char convertPionIntoColor(char p);
void sigPipe(int sig);

int main(int argc, char* argv[]){
	int i, j, fin = 0;
	int choix;
	
	if(argc == 1){
		printf("Le programme ne peut pas se lancer tout seul.\n");
		exit(-1);
	}
	else if(argc < 2){
		printf("Pas assez d'arguments.\n");
		exit(-1);
	}
	else if(argc > 2){
		printf("Trop d'arguments.\n");
		exit(-1);
	}
	
	signal(SIGPIPE, sigPipe);
	
	essaisDonnes = atoi(argv[1]);
	
	mkfifo(FIFO_SRV_DEC, FIFO_MODE);
	
	fdReadDec = open(FIFO_SRV_DEC, O_RDONLY);
	
	
	if(!initscr()){
		perror("initscr()");
		sleep(2000);
		exit(-1);
	}
	
	mainWin = newwin((essaisDonnes*2)+5, 22, 0, 0);
	box(mainWin, 0, 0);
	wrefresh(mainWin);
	
	decWin = subwin(mainWin, (essaisDonnes*2)+3, 13, 1, 1);
	box(decWin, 0, 0);
	wrefresh(decWin);
	
	invWin = subwin(mainWin, (essaisDonnes*2)+3, 7, 1, 14);
	box(invWin, 0, 0);
	wrefresh(invWin);
	
	if(!has_colors()){
		printf("Terminal décodeur ne supporte pas les couleurs.\n");
		getch();
		endwin();
		exit(-1);
	}
	
	start_color();
	
	init_color(PLAT_COL, 196, 121, 47);
	
	init_pair(1, COLOR_BLACK, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_RED);
	init_pair(3, COLOR_GREEN, COLOR_GREEN);
	init_pair(4, COLOR_YELLOW, COLOR_YELLOW);
	init_pair(5, COLOR_BLUE, COLOR_BLUE);
	init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(7, COLOR_CYAN, COLOR_CYAN);
	init_pair(8, COLOR_WHITE, COLOR_WHITE);
	
	init_pair(PLAT_PAIR, COLOR_BLACK, PLAT_COL);
	
	wbkgd(mainWin, COLOR_PAIR(PLAT_PAIR));
	wbkgd(decWin, COLOR_PAIR(PLAT_PAIR));
	wbkgd(invWin, COLOR_PAIR(PLAT_PAIR));
	
	wmove(decWin, 2, 2);
	for(j = 2; j <= (essaisDonnes*2)+1; j++){
		if(j % 2 == 0){
			for(i = 2; i <= 11; i++){
				if((i % 2) == 1) wprintw(decWin, "0");
				else wmove(decWin, j, i);
			}
		}
		else wmove(decWin, j+1, 1);
		
		wrefresh(decWin);
	}
	
	wmove(invWin, 2, 2);
	
	for(j = 2; j <= (essaisDonnes*2)+1; j++){
		if(j % 2 == 0){
			for(i = 1; i <= 5; i++){
				wmove(invWin, j, i);
				wprintw(invWin, "0");
			}
		}
		else wmove(invWin, j+1, 1);
		
		wrefresh(invWin);
	}
	
	
	wmove(decWin, 2, 2);
	
	for(j = 2; j <= (essaisDonnes*2)+1; j++){
		if(j % 2 == 0){
			for(i = 2; i <= 11; i++){
				if((i % 2) == 1){
					//choix = wgetch(decWin);
					read(fdReadDec, &choix, 1);
					if(choix >= '0' | choix <= '7'){
						wattrset(decWin, COLOR_PAIR((choix-48)+1));
						mvwprintw(decWin, j, i-1, "0");
						wattrset(decWin, COLOR_PAIR(PLAT_PAIR));
					}
					else i -= 2;
				}
				else wmove(decWin, j, i);
				
				wrefresh(decWin);
			}
			
			for(i = 1; i <= 5; i++){
				wmove(invWin, j, i);
				wattrset(invWin, COLOR_PAIR(PLAT_PAIR));
				//choix = wgetch(invWin);
				read(fdReadDec, &choix, 1);
				if(choix >= '0' | choix <= '2'){
					if(convertPionIntoColor(choix) == -1){
						mvwprintw(invWin, j, i, "0");
					}
					else {
						wattrset(invWin, COLOR_PAIR((convertPionIntoColor(choix)-48)+1));
						mvwprintw(invWin, j, i, "0");
					}
				}
				else i--;
				
				wrefresh(invWin);
			}
			
			read(fdReadDec, &fin, 1);
			if(fin == 1) break;
		}
		else wmove(decWin, j+1, 1);
		
		wrefresh(decWin);
	}
	
	read(fdReadDec, &fin, 1);
	delwin(decWin);
	delwin(invWin);
	delwin(mainWin);
	endwin();
	close(fdReadDec);
	return 0;
}


char convertPionIntoColor(char p){
	switch(p){
		case '1':
			return '7';
		case '2':
			return '0';
		default:
			return -1;
	}
}

void sigPipe(int sig){
	delwin(decWin);
	delwin(invWin);
	delwin(mainWin);
	endwin();
	close(fdReadDec);
	exit(1);
}
