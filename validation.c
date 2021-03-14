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
        printf("Ligne lue: %s", lignePCR);

        char resultat = lignePCR[strlen(lignePCR)-2]; //dernier caractère

        int longeur = strlen(lignePCR);

        char* timestamp = malloc(longeur-17-2); //tps validité (17-2 pour position initial - espace + resultat)
        
        for (int i =0; i < 16; i++){
            if (lignePCR[i] == emeteur[i]){ 
                if( i == 15 ){
                    int j=17;
                    //Récupération du délai
                    while(lignePCR[j]!= ' ' ){
                        timestamp[j-17] = lignePCR[j];

                        j = j+1;
                    }


                    /**
                     * 
                     *  CODE INTERMEDIAIRE dsl c deg T-T
                     * 
                    */
                   
                    //Recuperation de la date du jour (date de la demande)
                    time_t timestamp1 = time( NULL );
                    struct tm * pTime = localtime( & timestamp1 );

                    //char buffer[ MAX_SIZE ];
                    //strftime( buffer, MAX_SIZE, "%Y%m%d %H:%M:%S", pTime );
                    //printf( "Time : %s\n", buffer ); //à supprimer
                    

                    //test calcul jour deg faire switch ds le while
                    char jourTest[2];
                    jourTest[0] = timestamp[0];
                    jourTest[1] = timestamp[1];
                    int jourTestint = atoi(jourTest);
                    printf("jour du Test: %d \n", jourTestint);
                    int calculJour = pTime->tm_mday - jourTestint;
                    printf("calcul du jour : %d \n", calculJour);

                    char moisTest[2];
                    moisTest[0] = timestamp[2];
                    moisTest[1] = timestamp[3];
                    int moisTestint = atoi(moisTest);
                    printf("mois du Test: %d \n", moisTestint);
                    int calculMois = ((pTime->tm_mon+1) - moisTestint)*30; //*30 pour avoir le nbr de mois en jour
                    printf("calcul du mois : %d \n", calculMois);

                    char anneeTest[4];
                    anneeTest[0] = timestamp[4];
                    anneeTest[1] = timestamp[5];
                    anneeTest[2] = timestamp[6];
                    anneeTest[3] = timestamp[7];
                    int anneeTestint = atoi(anneeTest);
                    printf("annee du Test: %d \n", anneeTestint);
                    int calculAnnee = ((pTime->tm_year+1900) - anneeTestint)*360; //$360 pour avoir le nbr d'année en jours
                    printf("calcul de l'année : %d \n", calculAnnee); //ok

                    //Comparaison durée validité et durée entre la réalisation du test et aujourd'hui
                    int calculFinal = calculJour + calculMois + calculAnnee;
                    int valeurTest = atoi(valeur);
                    printf("Durée (en jours) entre la date du test et la date actuelle: %d \n", calculFinal);
                    if((-calculFinal) <= valeurTest){ //-CalculFinal car si on envoie une demande dont le test a été fait plus tard la valeur finale va être négative
                        printf("Validité du test correct \n");
                    }
                    else{
                        printf("Validité du test incorrect \n");
                    }

                    printf("timestamp: %s \n", timestamp);
                    printf("resultat : %c \n", resultat);


                    free(timestamp);
                }
            } 
            else{
                printf("Numéro du test inconnu. \n");
                break;
            }
            
        }
        
        

        /*if(strcmp(emeteur,emeteurPCR) == 0){
            |0001000000000000|Demande|13021|
            |0001000000000000|Demande|180| --> durée de validité de 180 jours
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