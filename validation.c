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

//#define MAX_SIZE 80

void usage(char * basename) { // DUPLICATION A CHANGER
    fprintf(stderr,
        "usage : %s [<programme 1> [<programme 2>]]\n",
        basename);
    exit(1);
}


int main(int argc, char* argv[])
{ 
    if (argc != 3) usage(argv[0]); // Test nombre arguments
    int argv0,argv1;
    sscanf (argv[0],"%d",&argv0); //conversion argv[0] en int
    sscanf (argv[1],"%d",&argv1); //conversion argv[1] en int

    dup2( argv0,0);        // Entrée     
    dup2( argv1,1);        // Sortie

    char* reponse = litLigne(0); // Lit l'entrée

    char emeteur[255], type[255], valeur[255];
    int msgDecoupe = decoupe(reponse, emeteur, type, valeur);

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        printf("print : Erreur de découpage!!\n");
        exit(1);
    }

    int df = open("Pcr.txt",O_RDONLY);

    char* lignePCR = litLigne(df);

    while(strcmp(lignePCR, "erreur") != 0 ){

        char* resultat = &(lignePCR[strlen(lignePCR)-2]); //dernier caractère
        int longueur = strlen(lignePCR);
        char* timestamp = malloc(longueur-17-2); //tps validité (17-2 pour position initial - espace + resultat)

        for (int i =0; i < 16; i++){
            if (lignePCR[i] == emeteur[i]){ 
                if( i == 15 ){
                    int j=17;
                    //Récupération du délai
                    while(lignePCR[j]!= ' ' ){
                        timestamp[j-17] = lignePCR[j];

                        j = j+1;
                    }

                    time_t now;
                    now = time(NULL);
                    

                    char* valide = "0";
                    int iValeur = atoi(valeur);
                    int iTimestamp = atoi(timestamp);
                    
                    if (now <= (iTimestamp + iValeur)){  
                        if(strcmp(resultat,"1\n") == 0){
                            valide = "1";
                        }
                    }
                    char *msg = message(emeteur,"Reponse", valide);
                    ecritLigne(1,msg);

                    free(timestamp);
                }
            } 
            else{
                //printf("Numéro du test inconnu. \n");
                break;
            }
            
        }
        
        

        /*if(strcmp(emeteur,emeteurPCR) == 0){
            |0001000000000000|Demande|13021|
        }*/

        lignePCR = litLigne(df);

    } 
    close(df);

    

        //bouvle tous les teste pcr pour trover le bon
        //check la date vailidité
        //stocker valeur du 0 ou 1
        //écrire un message n° test réponse + 0 ou 1

    

    // Lire Entrée (donc reception Demande)
    // Séparé le message (avoir le numéro)
    // Lire le nnuméro présent dans Pcr.txt
    // Vérifié validité
    // Ecrire la réponse sur la sortie

    
   return 0;
}