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


/**
 * Vérification du nombre d'argument passé en paramètre
 **/
void usage(char * basename) { 
    fprintf(stderr,
        "usage : %s [<programme 1> [<programme 2>]]\n",
        basename);
    exit(1);
}


int main(int argc, char* argv[])
{ 
    if (argc != 3) usage(argv[0]); // Test nombre arguments
    int argv0,argv1;
    sscanf (argv[1],"%d",&argv0); //conversion argv[0] en int
    sscanf (argv[2],"%d",&argv1); //conversion argv[1] en int

    fprintf(stderr,"les deux sorties : %d , %d\n",argv0,argv1);

    dup2( argv0,0);        // Redirection de l'entrée     
    dup2( argv1,1);        // Redirection de la sortie

    while(1){
        char* reponse = litLigne(0); // Lit l'entrée

        char emeteur[255], type[255], valeur[255];
        int msgDecoupe = decoupe(reponse, emeteur, type, valeur); //Découpe le message en 3 parties

        if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
            fprintf(stderr,"print : Erreur de découpage!!\n");
            exit(1);
        }

        int df = open("Pcr.txt",O_RDONLY); //Ouverture du fichier contenant les numéros de tests PCR

        char* lignePCR = litLigne(df); //Lis la ligne dans le descripteur de fichier

        while(strcmp(lignePCR, "erreur") != 0 ){ //Tant que nous n'avons pas finis de lire le fichier

            char* resultat = &(lignePCR[strlen(lignePCR)-2]); //dernier caractère
            int longueur = strlen(lignePCR);
            char* timestamp = malloc(longueur-19); //Temps de validité calcul pour trouver la longueur date (Longeur Ligne PCR - 16(n° PCR) - 2(2 espaces) -1 (Résultat test)) 

            for (int i =0; i < 16; i++){
                if (lignePCR[i] == emeteur[i]){ //Si le numéro de la Demande correspond au numéro du test PCR dans le fichier
                    if( i == 15 ){
                        int j=17;
                        
                        while(lignePCR[j]!= ' ' ){//Récupération du délai
                            timestamp[j-17] = lignePCR[j];
                            j = j+1;
                        }
                        time_t now;
                        now = time(NULL); //Récupération du temps actuelle en secondes depuis le 1er janvier 1970 à 00:00:00
                        
                        char* valide = "0"; //Demande valide
                        int iValeur = atoi(valeur); //Transformation de char* valeur  en int
                        int iTimestamp = atoi(timestamp);//Transformation de char* timestamp en int
                        
                        if (now <= (iTimestamp + iValeur)){ //Si la Demande est encore valide 
                            if(strcmp(resultat,"1\n") == 0){//Si la valeur de la réponse est 1
                                valide = "1";//La demande est valide
                            }
                        }
                        char *msg = message(emeteur,"Reponse", valide); //Création de la réponse
                        ecritLigne(1,msg); //Ecriture de la réponse dans le descripteur de fichier

                        free(timestamp);
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