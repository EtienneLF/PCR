#include <stdio.h>
#include <string.h>
#include "message.h"
#include "alea.h"
#include "lectureEcriture.h"

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <time.h>

/**
 * Création du message dans le format |Numéro_du_test|Type|Valeur|
 * @param num : numéro du test
 **/
char* createMsg(char* num){
    char time[255];

    aleainit();// Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(0,1209600)); //durée aléatoire Entre 0secondes et 14 jours

    char *msg = message(num, "Demande", time);

    return msg;
}

/**
 * Génère une ligne aléatoire à partir d'un descripteur de fichier
 * @param df: descripteur de fichier
 * @param num: numéro de la ligne
 **/
char* ligneAleatoire(int df, int num){
    char* lignePCR;

    for (int i =0; i<=num;i++){
        lignePCR = litLigne(df);
    }
    
    return lignePCR;
}

/**
 * Analyse de la réponse
 * Retourne 1 si test valide
 * Retourne 0 si test refusé
 * @param valeur: valeur de la fin du test
 **/
int analyseValeur(char* valeur)
{
    if(strcmp(valeur,"0")==0){//Le test PCR est refusé
        return 0;
    }
    
    else if(strcmp(valeur,"1")==0){//Le test PCR est validé
        return 1;
    }
    
    else{ //Si la valeur n'est pas 0 ou 1
        printf("Valeur non reconnue. \n");
        exit(1);
    }
}

/**
 * Compte le nombre de ligne dans un descripteur de fichier
 * @param nomFichier: descripteur de fichier 
 **/
int compteLigne(char * nomFichier){
    int df = open(nomFichier,O_RDONLY);
    char * ligne = litLigne(df);
    int temp = 0;
    while(strcmp(ligne, "erreur") != 0){
        temp +=1;
        ligne = litLigne(df);
    }
    close(df);
    return temp;
}

/**
 * Print l'erreur si le nombre d'argument passé est insuffisant
 * @param basename: argument
 **/
void usage(char * basename)
{ 
    fprintf(stderr,
        "usage : %s [<Descripteur fichier Entrée> [<Descripteur fichier Sortie>]\n",
        basename);
    exit(1);
}

int main(int argc, char* argv[])
{ 
    //Demande
    if (argc != 3) usage(argv[0]); // Test nombre arguments
    int argv1,argv2;
    sscanf (argv[1],"%d",&argv1); //conversion argv[0] en int
    sscanf (argv[2],"%d",&argv2); //conversion argv[1] en int

    //Compte le nombre de ligne dans le fichier
    int nbrLigne = compteLigne("Liste_test.txt");
    int fd = open("Liste_test.txt",O_RDONLY);
    //Cherche une ligne aléatoire dans le fichier
    aleainit();
    int aleaNum = alea(0,nbrLigne-1); 
    char * ligne = ligneAleatoire(fd, aleaNum); 
    
    //Création du message
    char* num = calloc(sizeof(char),16);
    for (int i =0; i < 16; i++){ //Récupération des 16 chiffres qui compose le numéro du test
        num[i] = ligne[i];
    }
    char* msg = createMsg(num);
    free(num); //Libération de la mémoire
    fprintf(stderr," Voici le message : %s", msg); // Affichage du message
    
    dup2( argv1,0);       // Redirection de l'entrée
    dup2( argv2,1);       // Redirection de la sortie

    ecritLigne(1,msg); //envoie le message

    //Réponse
    char* reponse = litLigne(0); // Lit la réponse

    char emeteur[255], type[255], valeur[255];
    int msgDecoupe = decoupe(reponse, emeteur, type, valeur); //Découpe du message en 3 parties

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        printf("print : Erreur de découpage!\n");
        exit(1);
    }
    if( analyseValeur(valeur)) { //Si la valeur est 1
        fprintf(stderr,"Validé\n");
    }
    else{ //Si la valeur est 0
        fprintf(stderr,"Refusé\n");
    }

    close(fd); //Fermeture du descripteur de fichier
    while(1); 

   return 0;
}
