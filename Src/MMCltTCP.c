#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <curses.h>

#include "mast_vars.h"


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


static int SockClient;
struct sockaddr_in adClientTCP, adServeurTCP;


int creerSocket(int, int *, struct sockaddr_in *);
void clientService(int, int, char**);

char *concat2(char *, char *);
char convertPionIntoColor(char);

void closeAfterKill(int sig);
void pipeSig(int sig);


int main(int argc, char *argv[]){
	int portTCP = 0;
	struct hostent *hp;
	
	if(argc != 3 || argc == 1){
		fprintf(stderr, "Usage : \"MMCltTCP <nom_machine> <n°port>\"\n");
		exit(2);
	}
	
	if((hp=gethostbyname(argv[1])) == NULL){
		fprintf(stderr, "Erreur nom machine serveur.\n");
		exit(2);
	}
	
	if((SockClient = creerSocket(SOCK_STREAM, &portTCP, &adClientTCP)) == -1){
		fprintf(stderr, "Création socket impossible.\n");
		exit(2);
	}
	
	signal(SIGINT, closeAfterKill);
	signal(SIGPIPE, pipeSig);
    
    system("clear");
	printf("Client de PID %d lancé\n\n", getpid());
	printf("Client sur le port %d\n\n", ntohs(adClientTCP.sin_port));
	adServeurTCP.sin_family = AF_INET;
	adServeurTCP.sin_port = htons(atoi(argv[2]));
	memcpy(&adServeurTCP.sin_addr.s_addr, hp->h_addr, hp->h_length);
	
	//Demande de connexion
	if(connect(SockClient, &adServeurTCP, sizeof(adServeurTCP)) == -1){
		fprintf(stderr, "Connexion impossible.\n");
		exit(2);
	}
	
	clientService(SockClient, argc-3, argv+3);
	
	return 0;
}

int creerSocket(int type, int *portTCP, struct sockaddr_in *ptr_adresse){
	int desc;
	int longueur = sizeof(struct sockaddr_in);
	
	desc = socket(AF_INET, type, 0);
	if(desc == -1){
		perror("Création socket impossible.");
		return(-1);
	}
	
	ptr_adresse->sin_family = AF_INET;
	ptr_adresse->sin_port = htons(*portTCP);
	ptr_adresse->sin_addr.s_addr = INADDR_ANY;
	if((bind(desc, (struct sockaddr *)ptr_adresse, longueur)) == -1){
		perror("Attachement socket impossible.");
		close(desc);
		exit(-1);
	}
	
	if(ptr_adresse != NULL)
		getsockname(desc, ptr_adresse, &longueur);
	
	return desc;
}

void clientService(int sock, int argc, char *argv[]){
	char pret;
	char combiEssayees[5][essaisDonnes], pionsInvest[5][essaisDonnes];
	
    int i, cpt = 0, col, choix, decGagne = 2, fin = 0;
	int essaiActuel = 1, fdWriteCod, fdWriteDec;
    int32_t ret;
	
	pid_t pidF, pidFi;
	
	struct stat st = {0};
	
	
	initscr();
	
	if(!has_colors()){
		printf("Ton terminal ne supporte pas les couleurs.\n");
		getch();
		raise(SIGINT);
	}
	
	start_color();
	use_default_colors();
	
	init_pair(1, COLOR_BLACK, -1);
	init_pair(2, COLOR_RED, -1);
	init_pair(3, COLOR_GREEN, -1);
	init_pair(4, COLOR_YELLOW, -1);
	init_pair(5, COLOR_BLUE, -1);
	init_pair(6, COLOR_MAGENTA, -1);
	init_pair(7, COLOR_CYAN, -1);
	init_pair(8, COLOR_WHITE, -1);
	
	box(stdscr, 0, 0);
	idlok(stdscr, TRUE);scrollok(stdscr, TRUE);
	
	printw("Client de PID %d lancé\n\n", getpid());
	refresh();
	
	printw("\nLe codificateur est en train de choisir la combinaison à te faire deviner.");
	refresh();
	
    for(i = 0; i < 5; i++){
        if(recv(sock, &combiGagnante[i], 1, 0) >= 0){
			//printw("\nTest de réception : %x en pos %d\n", combiGagnante[i], i+1);
			if(combiGagnante[i] != 0x00){
				printw("\nCouleur en position %d reçue.\n", i+1);
				refresh();
			}
			else {
				printw("\n\nLe serveur nous a quitté. Appuie sur une touche pour quitter.\n");
				getch();
				raise(SIGINT);
			}
		}
		else {
			perror("recv() ");
			raise(SIGINT);
		}
    }
	
	//printf("Ceci est un test de réception : %s\n\n", combiGagnante);
	printw("\nMaintenant il réfléchit à combien d'essais te donner.");
	refresh();
	if(recv(sock, &ret, sizeof(ret), 0) < 0){
		perror("recv() ");
		raise(SIGINT);
	}
	
    essaisDonnes = ntohl(ret);
	if(essaisDonnes == 0){
		printw("\n\nLe serveur nous a quitté. Appuie sur une touche pour quitter.\n");
		getch();
		raise(SIGINT);
	}
	
	printw("\nLe codificateur te donne %d essai(s). Es-tu prêt ? Tapes y (Y) ou n (N).\n> ", essaisDonnes);
	pret = getch();
	if(send(sock, &pret, 1, 0) < 0){
		perror("send() ");
		raise(SIGINT);
	}
	
	if(pret == 'y' | pret == 'Y'){
	    printw("\nBien allons-y. Appuie sur une touche pour commencer.\n");
		getch();
	}
	else if(pret == 'n' | pret == 'N'){
	    printw("\nTant pis. Appuie sur une touche pour quitter.\n\n");
		getch();
		raise(SIGINT);
	}
	
	
	if(stat("./FIFOs/", &st) < 0) mkdir("./FIFOs/", 0777);
	
	mkfifo(FIFO_CLT_COD, FIFO_MODE);
	
	mkfifo(FIFO_CLT_DEC, FIFO_MODE);
	
	pidF = fork();
	if(pidF < 0){
		perror("forkCod() ");
		exit(-1);
	}
	else if(pidF == 0){ // Fils 1
		int exitSt;
		char *command;
		
		command = concat2("./ClientCoder.sh ", combiGagnante);
		exitSt = system(command);
		
		free(command);
		exit(exitSt);
	}
	fdWriteCod = open(FIFO_CLT_COD, O_WRONLY);
	
	pidFi = fork();
	if(pidFi < 0){
		perror("forkDec() ");
		exit(-1);
	}
	else if(pidFi == 0){ // Fils 2
		int exitSt;
		char essaisStr[3], *command;
		
		snprintf(essaisStr, sizeof(essaisDonnes), "%d", essaisDonnes);
		
		command = concat2("./ClientDecoder.sh ", essaisStr);
		exitSt = system(command);
		
		free(command);
		exit(exitSt);
	}
	fdWriteDec = open(FIFO_CLT_DEC, O_WRONLY);
	
	
	while(essaiActuel <= essaisDonnes){
		clear();
		printw("Essai N.%d en cours...\n\n", essaiActuel);
		printw("Tu dois placer 5 pions. Le choix de couleurs se fait ainsi :\n\n");
		printw("0 = Noir\n1 = Rouge\n2 = Vert\n3 = Jaune\n4 = Bleu\n5 = Magenta\n6 = Cyan\n7 = Blanc\n\n");
		refresh();
		
		for(i = 0; i < 5; i++){
			printw("> ");
			choix = getch();
			if(choix < '0' | choix > '7'){
				printw("Ce n'est pas dans les choix. Réessaie.\n");
				refresh();
				i--;
			}
			else {
				combiEssayees[i][essaiActuel-1] = choix;
				if(send(sock, &choix, 1, 0) >= 0){
					if(write(fdWriteDec, &combiEssayees[i][essaiActuel-1], 1) < 0){
						printw("Client n'a pas pu écrire à la fenêtre décodeur.\n");
						getch();
						raise(SIGINT);
					}
					
					col = (choix - 48) + 1;
					attrset(COLOR_PAIR(col));
					printw("\n%s en %d\n", DOT, i+1);
					refresh();
					attrset(COLOR_PAIR(0));
				}
				else {
					perror("send()");
					raise(SIGINT);
				}
			}
		}

		printw("\n\nLe codificateur place les pions d'investigation.\n");
		refresh();
		
		for(i = 0; i < 5; i++){
			if(recv(sock, &pionsInvest[i][essaiActuel-1], 1, 0) < 0){
				perror("recv()");
				raise(SIGINT);
			}
			else {
				write(fdWriteDec, &pionsInvest[i][essaiActuel-1], 1);
				
                if(pionsInvest[i][essaiActuel-1] == 0x00){
                    printw("Le serveur nous a quitté.\n");
		            getch();
		            raise(SIGINT);
                }
                else if(pionsInvest[i][essaiActuel-1] == '0'){
					printw("Pion %d : Y'en a pas\n", i+1);
					refresh();
				}
				else {
					attrset(COLOR_PAIR((convertPionIntoColor(pionsInvest[i][essaiActuel-1]) - 48) + 1));
					printw("Pion %d : %s\n", i+1, DOT);
					refresh();
					attrset(COLOR_PAIR(0));
				}
				
				if(pionsInvest[i][essaiActuel-1] == '2') cpt++;
			}
		}
		
		printw("\n");
		
		if(cpt == 5) {
			decGagne = 1;
			fin = 1;
			break;
		}
		else {
			printw("\nDommage, c'est pas la bonne combinaison. ");
			cpt = 0;
			essaiActuel++;
			if(essaiActuel > essaisDonnes) {
				decGagne = 0;
				fin = 1;
				break;
			}
		}
		
		write(fdWriteDec, &fin, 1);
		write(fdWriteCod, &fin, 1);
		printw("Appuie sur une touche pour continuer.\n");
		getch();
		refresh();
		clear();
	}
	
	write(fdWriteCod, &fin, 1);
	
	switch(decGagne){
		case 1:
			printw("\nChapeau ! Combinaison devinée en %d essais.\n", essaiActuel);
			refresh();
			break;
		case 0:
			printw("\nT'auras peut-être plus de chances à la prochaine.\n");
			refresh();
			break;
		default:
			printw("\nAh, le serveur nous a quitté.\n");
			refresh();
			break;
	}
	
	printw("Appuie sur n'importe quelle touche pour quitter.\n");
	getch();
	write(fdWriteCod, &fin, 1);
	write(fdWriteDec, &fin, 1);
	unlink(FIFO_CLT_COD);
	unlink(FIFO_CLT_DEC);
    rmdir("./FIFOs");
	wait(NULL);
	clear();
	endwin();
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



void closeAfterKill(int sig){
	printw("Fermeture forcée.\n\n");
	close(SockClient);
	wait(NULL);
	clear();
	endwin();
	unlink(FIFO_CLT_COD);
	unlink(FIFO_CLT_DEC);
    rmdir("./FIFOs");
	exit(1);
}

void pipeSig(int sig){
	printw("\nSIGPIPE reçu : le serveur nous a quitté.\n\n");
	getch();
	close(SockClient);
	wait(NULL);
	clear();
	endwin();
	unlink(FIFO_CLT_COD);
	unlink(FIFO_CLT_DEC);
    rmdir("./FIFOs");
	exit(1);
}
