# PCR
# Projet PCR pour EL-3032

## Mode d’emploi 
### Compilation de l’application
Afin de compiler l’application il suffit de rentrer la commande make dans un terminal positionné dans le dossier PCR. Afin de nettoyer tous les fichiers exécutables et fichiers objet il faut utiliser la commande make cleanall.

### Fichiers indispensables
Pour utiliser l’application il faut au préalable posséder au moins 3 fichiers :
- Un fichier de configuration (exemple présent dans l'archive : *centre_archivage.txt*). Ce fichier conservera la liste des différents serveurs d’acquisition a pour forme :  *Numéro_du_Centre(4chiffres) Nombre_de_Terminaux Fichier_test_PCR Nom_Centre*.
- Un fichier contenant la liste de tous les numéros de test PCR possédant l'intitulé : *Liste_test.txt*. Ce fichier est utile pour la démonstration d’un tirage aléatoire d’un test PCR.
- Au moins un fichier contenant les résultats des tests PCR. Les fichiers seront de la forme *Numéro_du_Test_PCR Date_Du_Test (TimeStamp Unix) Résultat (1 = positif, 0 = négatif)*. Ces fichiers sont utilisés dans le fichier de configuration (*Fichier_test_PCR*). Il est conseillé de posséder autant de fichier de résultat que de centres d’acquisition (exemple présent dans l'archive : *Pcr1.txt* et *Pcr2.txt*).

### Exécution de l’application et fonctionnement de l’application
Il vous suffira d’exécuter la commande suivante dans le terminal : “./interarchives [<Taille mémoire>] [<Nom fichier configuration>]”. Ce terminal sert d’interface à InterArchive, toutes les requêtes seront affichées avec la destination et la source si c’est une demande.
Plusieurs fenêtres de xTerm vont s’afficher :
Les fenêtres Acquisition, préciserons le numéro, le nom et le fichier dans lequel sont stockés les résultats du centre d’acquisition. Comme pour InterArchive le terminal affiche toutes les requêtes avec la destination et la source si c’est une demande.
Les fenêtres Terminal, affichent une demande aléatoire tirée de *Liste_test.txt* ainsi que la réponse reçue, puis attends la rentrée manuelle d’une nouvelle demande de test.
Pour terminer l’application, il faut fermer les terminaux, soit en tapant exit, soit en fermant la fenêtre. Puis une fois tous les terminaux d’un serveur Acquisition fermé, il faut aussi taper exit dans le bash, afin de finir le processus Validation et libérer la mémoire. Une fois tous les serveurs d’acquisition fermés, le processus d’Interarchive se fermera tout seul.

- alea.(c,h) : generateur aleatoire - teste dans TestMessage
