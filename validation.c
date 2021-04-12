#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "lectureEcriture.h"
#include "message.h"

#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Valeur de validation du message de réponse
char* valide = "0";

/**
 * Print l'erreur si le nombre d'argument passé est insuffisant
 * @param basename: argument
 **/
void usage(char * basename) { 
    fprintf(stderr,
        "usage : %s [<Descripteur fichier Entrée> [<Descripteur fichier Sortie>] [<Nom fichier>]\n",
        basename);
    exit(1);
}

/**
 * Change la valeur de valide si la valeur de la réponse est 1
 * @param timestamp: durée de validité
 * @param resultat: resultat lu dans le fichier
 * @param valeurReponse: valeur de réponse (0 ou 1)
 * */
void validation1(char * timestamp, char * resultat, char* valeurReponse){
    time_t now;
    now = time(NULL); //Récupération du temps actuelle en secondes depuis le 1er janvier 1970 à 00:00:00
    
    int iValeur = atoi(valeurReponse); //Transformation de char* valeur  en int
    int iTimestamp = atoi(timestamp);//Transformation de char* timestamp en int

    fprintf(stderr, "Now : %li <= Test (%i) + timestamp (%i) %i\n", now,iTimestamp,iValeur, iValeur+iTimestamp);
    
    if (now <= (iTimestamp + iValeur)){ //Si la Demande est encore valide 
        if(strcmp(resultat,"0\n") == 0){//Si la valeur de la réponse est 1
            valide = "1";//La demande est valide
        }
    }
}

int main(int argc, char* argv[])
{ 
    if (argc != 4) usage(argv[0]); // Test nombre arguments
    int argv1 = atoi(argv[1]);
    int argv2 = atoi(argv[2]);
    char * nom_fichier = argv[3];

    fprintf(stderr,"les deux sorties : %d , %d\n",argv1,argv2);

    dup2( argv1,0);        // Redirection de l'entrée     
    dup2( argv2,1);        // Redirection de la sortie

    while(1){//Lis en continu les demandes 
        char* reponse = litLigne(0); // Lit l'entrée

        char emeteur[255], type[255], valeur[255];
        int msgDecoupe = decoupe(reponse, emeteur, type, valeur); //Découpe le message en 3 parties

        if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
            fprintf(stderr,"print : Erreur de découpage!!\n");
            exit(1);
        }

        int df = open(nom_fichier,O_RDONLY); //Ouverture du fichier contenant les numéros de tests PCR

        char* lignePCR = litLigne(df); //Lis la ligne dans le descripteur de fichier

        while(strcmp(lignePCR, "erreur") != 0 ){ //Tant que nous n'avons pas finis de lire le fichier

            char* resultat = &(lignePCR[strlen(lignePCR)-2]); //dernier caractère
            int longueur = strlen(lignePCR); //longueur de la ligne 
            char* timestamp = malloc(longueur-19); //Temps de validité: calcul pour trouver la longueur date (Longeur Ligne PCR - 16(n° PCR) - 2(2 espaces) -1 (Résultat test)) 

            for (int i =0; i < 16; i++){//Pour chaque numéro du test
                if (lignePCR[i] == emeteur[i]){ //Si le numéro de la Demande correspond au numéro du test PCR dans le fichier
                    if( i == 15 ){
                        int j=17;
                        
                        while(lignePCR[j]!= ' ' ){//Récupération du délai
                            timestamp[j-17] = lignePCR[j];
                            j = j+1;
                        }

                        validation1(timestamp, resultat,valeur); //modification de valide si la valeur du test est 1
                        char *msg = message(emeteur,"Reponse", valide); //Création de la réponse
                        ecritLigne(1,msg); //Ecriture de la réponse dans le descripteur de fichier
                        free(timestamp); //Libération de la mémoire
                    }
                } 
                else{ //Si le numéro de la demande ne correspond pas à la ligne lue
                    break;
                }
            }
            lignePCR = litLigne(df); //Prochaine ligne
        } 
        close(df); //Fermeture du descripteur de fichier
    }
   return 0;
}