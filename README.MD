# Installation


## Prérequis


Pour exécuter ces programmes, il faut la librairie NCurses:


### Ubuntu / Debian


>sudo apt-get install libncurses5-dev libncursesw5-dev


### CentOS / RHEL / Fedora 21


>sudo yum install ncurses-devel


### Fedora 22+


>sudo dnf install ncurses-devel


## Compilation


Il suffit d'exécuter le fichier *compileAll.sh* dans le dossier **Src**, là où se trouve le code source. Le code compilé sortira dans les dossiers correspondant à ce que le client et le serveur doivent contenir.


# Exécution


Les fichiers *ClientCoder.sh*, *ClientDecoder.sh*, *ServerCoder.sh*, *ServerDecoder.sh* sont nécessaires au bon fonctionnement du programme. **Je ne suis en aucun cas responsable de toute suppression.**

Le processus de fonctionnement se déroule ainsi :
- Serveur exécute *MMSrvTCP* avec le port choisi comme argument
- Client exécute *MMCltTCP* avec l'adresse IP du serveur et le port choisi par le serveur comme arguments.
- Les programmes, une fois les codificateur et décodeur prêts, vont ensuite exécuter leur *MMCod* et *MMDec* respectifs, prorammes d'affichage du plateau.
- La partie terminée, le Serveur revient à la recherche de client, tandis que le programme Client finit son exécution.


# Règles


Dans ce jeu, les combinaisons contiennent uniquement 5 fiches [^1] et le jeu de couleurs contient uniquement 8 couleurs [^2].

Ici, le Codificateur est le Serveur, et le Décodeur est le Client.

Une fois le Client connecté au Serveur, la partie se déroule ainsi :
- Le codificateur choisit, à l'aide d'un guide "caractère -> couleur" affiché, la combinaison à faire deviner au décodeur.
- Il choisit ensuite le nombre d'essais à donner à celui-ci. **Il ne peut donner à son décodeur ni 0 essais ni un nombre d'essais supérieur à 12.** Il est toutefois possible d'accorder seulement 1 essai.
- Le décodeur doit maintenant indiquer s'il se sent prêt à relever le défi. Auquel cas la partie commence. Sinon, le programme décodeur se termine et le codificateur s'en va rechercher un autre décodeur.
- Le décodeur prêt, les joueurs appuient sur une touche pour commencer la partie et afficher le plateau. Le plateau du codificateur, *du côté du codificateur*, affiche la combinaison gagnante, tandis que ce même plateau *du côté du décodeur* affiche que des pions blancs, jusqu'à la fin de la partie. Le plateau du décodeur est le même de chaque côté : il affiche les combinaisons d'essais du décodeur et les pions d'investigation placés par le codificateur.
- Dans ce jeu de Mastermind, le programme est réalisé de telle sorte à ce que les échanges entre codificateur et décodeur soit mis en avant. *De ce fait, les pions d'investigation sont placés par le codificateur et non par le programme. Le décodeur gagne la partie si et seulement si le codificateur place 5 pions d'investigation noirs à la suite d'un essai.*
- La partie se termine lorsque le codificateur place 5 pions d'investigation noirs ou lorsque le nombre d'essais donnés par le codificateur est dépassé par le décodeur. Le plateau du codificateur *du côté du décodeur* affiche alors la combinaison à deviner, qu'il ait gagné (par les 5 pions d'investigation noirs) ou perdu. *Le programme du décodeur se termine, alors que le programme du codificateur retourne à la recherche d'un autre décodeur.*


# Problèmes rencontrés lors de la réalisation


En utilisant *execlp()*, seuls les programmes d'affichage du plateau du codificateur se lançaient. Ce problème a été résolu en utilisant *system()* et l'intermédiaire d'un fichier de commandes *SH* pour chaque programme d'affichage des plateaux.





[^1]: Règle provenant d'une ancienne version du jeu de société.
[^2]: Les 8 couleurs correspondent aux 8 couleurs de base disponibles dans la librairie NCurses.