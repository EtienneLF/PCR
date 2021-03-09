#include <stdio.h>
#include <string.h>
#include "message.h"
#include "alea.h"
#include "lectureEcriture.h"

#include <stdlib.h>
#include <unistd.h>

#include <time.h>


char* createMsg()
{
    char time[255];

    aleainit();// Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1,50000)); //durée aléatoire

    char *msg = message("0001000000000000", "Demande", time);

    return msg;
}

/**
 * Analyse de la réponse
 * 1 test valide
 * 0 test refusé
*/
int analyseValeur(char* valeur /*valeur de fin du test*/)
{
    //Le test PCR est refusé
    if(strcmp(valeur,"0")==0){
        return 0;
    }
    //Le test PCR est validé
    else if(strcmp(valeur,"1")==0){
        return 1;
    }
    
    else{
        printf("print : Valeur non reconnue. \n");
        exit(1);
    }
}

void usage(char * basename) // Print une erreur si pas assez d'argument mis
{ 
    fprintf(stderr,
        "usage : %s [<programme 1> [<programme 2>]]\n",
        basename);
    exit(1);
}

int main(int argc, char* argv[])
{ 
    char * Message = createMsg(); // Création du Message
    printf("print : Voici le message : %s", Message); // Affichage du message

    if (argc != 3) usage(argv[0]); // Test nombre arguments
    int argv0,argv1;
    sscanf (argv[0],"%d",&argv0); //conversion argv[0] en int
    sscanf (argv[1],"%d",&argv1); //conversion argv[1] en int

    dup2( argv0,0);        // Entrée     
    dup2( argv1,1);        // Sortie

    ecritLigne(1,Message); //envoie le message

    char* reponse = litLigne(0); // Lit la réponse

    //dup2(1,1); 

    char emeteur[255], type[255], valeur[255];
    int msgDecoupe = decoupe(reponse, emeteur, type, valeur);

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        printf("print : Erreur de découpage!!\n");
        exit(1);
    }

    if( analyseValeur(valeur)) {
        printf("Validé\n");
    }
    else{
        printf("Refusé\n");
    }
   return 0;
}