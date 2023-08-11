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

WINDOW *mainwin;
int fdReadCod;

void sigPipe(int sig);

int main(int argc, char* argv[]){
	int fin = 0, i, nb, cpt = 0;
	
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
	
	for(i = 0; i < 5; i++) combiGagnante[i] = argv[1][i];
	
	mkfifo(FIFO_CLT_COD, FIFO_MODE);
	
	fdReadCod = open(FIFO_CLT_COD, O_RDONLY);
	
	
	initscr();
	
	mainwin = newwin(5, 13, 0, 0);
	box(mainwin, 0, 0);
	wrefresh(mainwin);
	
	if(!has_colors()){
		printf("Terminal codeur (client) ne supporte pas les couleurs.\n");
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
	
	wbkgd(mainwin, COLOR_PAIR(PLAT_PAIR));
	wmove(mainwin, 2, 3);
	for(i = 2; i <= 11; i++){
		if((i % 2) == 1){
			wattrset(mainwin, COLOR_PAIR(8));
			mvwprintw(mainwin, 2, i-1, "0");
			wrefresh(mainwin);
			wattrset(mainwin, COLOR_PAIR(PLAT_PAIR));
		}
		else {
			wmove(mainwin, 2, i);
			wrefresh(mainwin);
		}
	}
	
	wmove(mainwin, 2, 3);
	
	while(fin == 0){
		read(fdReadCod, &fin, 1);
		if(fin == 1) break;
	}
	
	for(i = 2; i <= 11; i++){
		if((i % 2) == 1){
			wattrset(mainwin, COLOR_PAIR((combiGagnante[cpt]-48)+1));
			mvwprintw(mainwin, 2, i-1, "0");
			wrefresh(mainwin);
			wattrset(mainwin, COLOR_PAIR(PLAT_PAIR));
			cpt++;
		}
		else {
			wmove(mainwin, 2, i);
			wrefresh(mainwin);
		}
	}
	
	cpt = 0;
	
	while((nb = read(fdReadCod, &fin, 1)) <= 0);
	
	delwin(mainwin);
	endwin();
	close(fdReadCod);
	return nb;
}


void sigPipe(int sig){
	delwin(mainwin);
	endwin();
	close(fdReadCod);
	exit(1);
}
