#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <curses.h>

#include "mast_vars.h"

#define getName(var) #var


//#define EXMNRM "\x1B[0;3"
//#define RSTCOL "\x1B[0;0m"
//#define BLKCOL "\x1B[0;30m"
//#define REDCOL "\x1B[0;31m"
//#define GRNCOL "\x1B[0;32m"
//#define YLWCOL "\x1B[0;33m"
//#define BLUCOL "\x1B[0;34m"
//#define MAGCOL "\x1B[0;35m"
//#define CYACOL "\x1B[0;36m"
//#define WHTCOL "\x1B[0;37m"
#define DOT "\u2b24"


static int SockEcoute, SockService;
WINDOW *pereWin, *filsWin, *debugWin;


int creerSocket(int, int *, struct sockaddr_in *);

void clean_stdin();

char *concat2(char *, char *);
char convertPionIntoColor(char);

void segVSig(int sig);
void intSig(int sig);
void pipeSig(int sig);
void closeEverything();

int main(int argc, char *argv[]){
	struct sockaddr_in adresse;
	struct stat st = {0};
	
	int fin = 0;
	char pret;
	char combiEssayees[5][essaisDonnes], pionsInvest[5][essaisDonnes];
	int portTCP, lg, choix, res, col, cpt = 0, i, codGagne = 2, essaiActuel = 1;
	int fdWriteCod, fdWriteDec;
    int32_t it;
	
	pid_t pidF, pidFi;
	
	
    if(argc != 2 || argc == 1){
	    fprintf(stderr, "Usage : MMSrvTCP <n°port>");
	    exit(2);
    }

	signal(SIGINT, intSig);
	signal(SIGSEGV, segVSig);
	signal(SIGPIPE, pipeSig);

    portTCP=atoi(argv[1]);

	lg=sizeof(adresse);

    if((SockEcoute=creerSocket(SOCK_STREAM, &portTCP, &adresse))==-1){
	    perror("Erreur creerSocket");
	    exit(1);
    }

    if(listen(SockEcoute, 5)==-1){
	    perror("Erreur listen");
	    exit(2);
    }
	
	system("clear");
	printf("Serveur de PID %d lancé\n", getpid());
	
	initscr();
	if(!has_colors()){
		printf("Ton terminal ne supporte pas les couleurs.\n");
		getch();
		endwin();
		exit(1);
	} else {
		start_color();
		use_default_colors();
	}
	
	init_pair(1, COLOR_BLACK, -1);
	init_pair(2, COLOR_RED, -1);
	init_pair(3, COLOR_GREEN, -1);
	init_pair(4, COLOR_YELLOW, -1);
	init_pair(5, COLOR_BLUE, -1);
	init_pair(6, COLOR_MAGENTA, -1);
	init_pair(7, COLOR_CYAN, -1);
	init_pair(8, COLOR_WHITE, -1);
	refresh();
	
	pereWin = subwin(stdscr, LINES, COLS / 2, 0, COLS / 2); // A décommenter quand debugWin n'est pas utilisé
	//pereWin = subwin(stdscr, LINES / 2, COLS / 2, 0, COLS / 2); // A décommenter quand il l'est
	scrollok(pereWin, TRUE);
	
	//debugWin = subwin(stdscr, LINES / 2, COLS / 2, LINES / 2, COLS / 2); // Fenêtre de debug des variables
	
	wrefresh(pereWin);
	//wrefresh(debugWin);

    wprintw(pereWin, "Serveur de PID %d lancé\n", getpid());

	while(1){
		wclear(pereWin);
		box(pereWin, 0, 0);
		//box(debugWin, 0, 0);
		
		wprintw(pereWin, "En attente d'un client.\n\n");
        wrefresh(pereWin);

		SockService=accept(SockEcoute, &adresse, &lg);
		wprintw(pereWin, "Connexion acceptée par %s\n", inet_ntoa(adresse.sin_addr));
        wrefresh(pereWin);
		
		if(fork() == 0){ // Fils
			close(SockEcoute);
			
			
			filsWin = subwin(stdscr, LINES, COLS / 2, 0, 0);
			box(filsWin, 0, 0);
			idlok(filsWin, TRUE);scrollok(filsWin, TRUE);
			wrefresh(filsWin);
			
			wprintw(filsWin, "Tu dois placer 5 pions. Le choix de couleurs se fait ainsi :\n\n");
			wprintw(filsWin, "0 = Noir\n1 = Rouge\n2 = Vert\n3 = Jaune\n4 = Bleu\n5 = Magenta\n6 = Cyan\n7 = Blanc\n\n");
			
			for(i = 0; i < 5; i++){
				wprintw(filsWin, "> ");
				choix = wgetch(filsWin);
				//wclear(debugWin);
				//wprintw(debugWin, "Variable: %s\nHex: %x\nDec: %d\nChar: %c\n", getName(choix), choix, choix, choix);
				//wrefresh(debugWin);

				if(choix < '0' | choix > '7'){
					if(choix == -1){
						wprintw(filsWin, "Le client nous a quitté.\n");
						wgetch(filsWin);
					    raise(SIGINT);
					}
					
					wprintw(filsWin, "Ce n'est pas dans les choix. Réessaie.\n");
					wrefresh(filsWin);
					i--;
				}
				else {
					combiGagnante[i] = choix;
					if(send(SockService, &choix, 1, 0) >= 0){
						col = (choix - 48) + 1;
						wattrset(filsWin, COLOR_PAIR(col));
					    wprintw(filsWin, "\n%s en %d\n", DOT, i+1);
						wrefresh(filsWin);
						wattrset(filsWin, COLOR_PAIR(0));
					}
					else {
					    perror("send()");
						raise(SIGINT);
					}
				}
			}
			
			wprintw(filsWin, "\n\n");
			
			essaisDonnes = 0;
			
			while(essaisDonnes < 1 | essaisDonnes > 12) {
				wprintw(filsWin, "\nCombien veux-tu donner d'essais à ton décodeur ? 12 maxi.\n> ");
				wrefresh(filsWin);
				
				res = wscanw(filsWin, "%d", &essaisDonnes);
				//wclear(debugWin);
				//wprintw(debugWin, "Variable: %s\nHex: %x\nDec: %d\nChar: %c\n", getName(essaisDonnes), essaisDonnes, essaisDonnes, essaisDonnes);
				//wrefresh(debugWin);
				
				if(res > 0){
					if(essaisDonnes > 12){
						wprintw(filsWin, "C'est trop. Réessaie.\n\n");
						wrefresh(filsWin);
					}
					else if(essaisDonnes < 1){
						wprintw(filsWin, "Bien tenté, mais tu ne peux pas ne pas lui donner d'essais. Réessaie.\n\n");
						wrefresh(filsWin);
					}
				}
				else if(res == ERR){
					wprintw(filsWin, "Y'a rien, là. Réessaie.\n\n");
					wrefresh(filsWin);
				}
				else if(res == 0){
					wprintw(filsWin, "C'est quoi, ça ? Je lis pas ça, moi. Réessaie.\n\n");
					wrefresh(filsWin);
				}
			}

			it = htonl(essaisDonnes);
			if(send(SockService, &it, sizeof(it), 0) < 0){
				perror("send() ");
				raise(SIGINT);
			}

			wprintw(filsWin, "Le décodeur se prépare...\n");
			wrefresh(filsWin);
			if(recv(SockService, &pret, 1, 0) < 0){
				perror("recv() ");
				raise(SIGINT);
			}
			
			//wclear(debugWin);
			//wprintw(debugWin, "Variable: %s\nHex: %x\nDec: %d\nChar: %c\n", getName(pret), pret, pret, pret);
			//wrefresh(debugWin);

			if(pret == 'y' | pret == 'Y'){
				wprintw(filsWin, "Le décodeur est prêt, c'est parti. Appuie sur une touche pour commencer.\n");
				wgetch(filsWin);
			}
			else if(pret == 'n' | pret == 'N' | pret == 0x01){
				wprintw(filsWin, "Ah non, il est parti. Appuie sur une touche pour quitter.\n\n");
				wgetch(filsWin);
				raise(SIGINT);
			}
			
			
			if(stat("./FIFOs", &st) < 0) mkdir("./FIFOs", 0770);
			
			
			mkfifo(FIFO_SRV_COD, FIFO_MODE);
			
			mkfifo(FIFO_SRV_DEC, FIFO_MODE);
			
			pidF = fork();
			if(pidF < 0){
				perror("forkCod() ");
				raise(SIGINT);
			}
			if(pidF == 0){ // Fils du fils 1
				int exitSt;
				char *command;
				
				command= concat2("./ServerCoder.sh ", combiGagnante);
				exitSt = system(command);
				
				free(command);
				exit(exitSt);
			}
			fdWriteCod = open(FIFO_SRV_COD, O_WRONLY);
			
			pidFi = fork();
			if(pidFi < 0){
				perror("forkDec() ");
				raise(SIGINT);
			}
			if(pidFi == 0){ // Fils du fils 2
				int exitSt;
				char essaisStr[3], *command;
				
				snprintf(essaisStr, sizeof(essaisDonnes), "%d", essaisDonnes);
				
				command = concat2("./ServerDecoder.sh ", essaisStr);
				exitSt = system(command);
				
				free(command);
				exit(exitSt);
			}
			fdWriteDec = open(FIFO_SRV_DEC, O_WRONLY);
			
			
			while(essaiActuel <= essaisDonnes){
				wclear(filsWin);
				
				wprintw(filsWin, "Essai N.%d en cours...\n\n", essaiActuel);
				wrefresh(filsWin);

				for(i = 0; i < 5; i++){
					if(recv(SockService, &combiEssayees[i][essaiActuel-1], 1, 0) < 0){
						perror("recv() ");
						raise(SIGINT);
					}
                    if(combiEssayees[i][essaiActuel-1] == 0x01){
                        wprintw(filsWin, "Le client nous a quitté. Appuie sur une touche pour quitter.\n");
						wgetch(filsWin);
				        raise(SIGINT);
                    }
					
					if(write(fdWriteDec, &combiEssayees[i][essaiActuel-1], 1) < 0){
						wprintw(filsWin, "Serveur n'a pas pu écrire à la fenêtre décodeur.\n");
						wgetch(filsWin);
						raise(SIGINT);
					}
					
					col = (combiEssayees[i][essaiActuel-1] - 48) + 1;
					wattrset(filsWin, COLOR_PAIR(col));
					wprintw(filsWin, "%s ", DOT);
					wrefresh(filsWin);
					wattrset(filsWin, COLOR_PAIR(0));
				}

				wprintw(filsWin, "\n\nLe placement des pions d'investigation se fait ainsi :\n");
				wprintw(filsWin, "0 = Rien (mauvaise couleur, mauvais endroit)\n1 = Blanc (bonne couleur, mauvais endroit)\n2 = Noir (bonne couleur, bon endroit)\n\n");
				wprintw(filsWin, "C'est toi qui les place. Tu n'es pas obligé de les placer selon les positions, ");
				wattrset(filsWin, COLOR_PAIR(2));
				wprintw(filsWin, "mais tu te dois d'être honnête.\n\n");
				wattrset(filsWin, COLOR_PAIR(0));
				
				wrefresh(filsWin);

				for(i = 0; i < 5; i++){
					wprintw(filsWin, "> ");
					choix = wgetch(filsWin);
					if(choix < '0' | choix > '2'){
						wprintw(filsWin, "\nCe n'est pas dans les choix. Réessaie.\n");
						wrefresh(filsWin);
						i--;
					}
					else {
						pionsInvest[i][essaiActuel-1] = choix;
						if(send(SockService, &choix, 1, 0) < 0){
							perror("send()");
							raise(SIGINT);
						}
						else {
							write(fdWriteDec, &pionsInvest[i][essaiActuel-1], 1);
							
							if(choix == '0'){
								wprintw(filsWin, "\nRien en %d\n", i+1);
								wrefresh(filsWin);
							}
							else {
								col = (convertPionIntoColor(pionsInvest[i][essaiActuel-1]) - 48) + 1;
								wattrset(filsWin, COLOR_PAIR(col));
								wprintw(filsWin, "\n%s en %d\n", DOT, i+1);
								wrefresh(filsWin);
								wattrset(filsWin, COLOR_PAIR(0));
							}

							if(pionsInvest[i][essaiActuel-1] == '2') cpt++;
						}
					}
					
					//wclear(debugWin);
					//wprintw(debugWin, "Variable: %s\nHex: %x\nDec: %d\nChar: %c\n", getName(choix), choix, choix, choix);
					//wrefresh(debugWin);
				}

				if(cpt == 5) {
					codGagne = 0;
					fin = 1;
					break;
				}
				else {
					cpt = 0;
					essaiActuel++;
					if(essaiActuel > essaisDonnes) {
						codGagne = 1;
						fin = 1;
						break;
					}
					write(fdWriteDec, &fin, 1);
					
					wprintw(filsWin, "\nIl a pas encore trouvé. Appuie sur une touche pour continuer.\n");
					wgetch(filsWin);
				}
				
			}

			switch(codGagne){
				case 1:
					wprintw(filsWin, "\n\nC'est gagné ! Il a utilisé tous ses essais.\n");
					wrefresh(filsWin);
					break;
				case 0:
					wprintw(filsWin, "\n\nIl a réussi en %d essais, dommage.\n", essaiActuel);
					wrefresh(filsWin);
					break;
				default:
					wprintw(filsWin, "\nAh ben le client est parti.\n");
					wrefresh(filsWin);
					break;
			}

			wprintw(filsWin, "Appuie sur n'importe quelle touche pour quitter cette partie et revenir à la recherche de client.\n");
			wgetch(filsWin);
			write(fdWriteCod, &fin, 1);
			write(fdWriteDec, &fin, 1);
			wclear(filsWin);
			delwin(filsWin);
			exit(0);
		}

		wait(NULL);
		clear();
		close(SockService);
	}
}

int creerSocket(int type, int *portTCP, struct sockaddr_in *ptr_adresse){
	int desc;
	int longueur = sizeof(struct sockaddr_in);

	desc = socket(AF_INET, type, 0);
	if(desc==-1){
		perror("Erreur création de socket");
		return(-1);
	}

	ptr_adresse->sin_family = AF_INET;
	ptr_adresse->sin_port = htons(*portTCP);
	ptr_adresse->sin_addr.s_addr = INADDR_ANY;
	if((bind(desc, (struct sockaddr *)ptr_adresse, longueur))==-1){
		perror("Erreur bind");
		close(desc);
		exit(-1);
	}

	if(ptr_adresse != NULL)
		getsockname(desc, ptr_adresse, &longueur);

	return desc;
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


char *concat2(char *s1, char *s2){
	char *result = malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}


void segVSig(int sig){
	printf("Erreur de segmentation.\n");
	closeEverything();
}

void intSig(int sig){
	printf("Commande d'interruption reçue.\n");
	closeEverything();
}

void pipeSig(int sig){
	printf("\nSIGPIPE reçu : le client nous a quitté.\n");
	wait(NULL);
	close(SockService);
	delwin(filsWin);
	clear();
    refresh();
}

void closeEverything(){
	printf("Fermeture forcée du serveur.\n\n");
	close(SockService);
	close(SockEcoute);
	delwin(filsWin);
	refresh();
	wait(NULL);
	//delwin(debugWin);
	refresh();
	delwin(pereWin);
	refresh();
	clear();
	endwin();
	unlink(FIFO_SRV_COD);
	unlink(FIFO_SRV_DEC);
    rmdir("./FIFOs");
	exit(1);
}
