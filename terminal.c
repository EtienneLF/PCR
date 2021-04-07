#include <stdio.h>
#include <string.h>
#include "message.h"
#include "alea.h"
#include "lectureEcriture.h"

#include <stdlib.h>
#include <unistd.h>

#include <time.h>

/**
 * Création du message dans le format |Numéro_du_test|Type|Valeur|
 **/
char* createMsg()
{
    char time[255];

    aleainit(); // Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1,50000)); //durée aléatoire

    char *msg = message("0001000000000000", "Demande", time); //Création d'une demande avec une durée de validité aléatoire

    return msg;
}

/**
 * Analyse de la réponse
 * Retourne 1 si test valide
 * Retourne 0 si test refusé
*/
int analyseValeur(char* valeur /*valeur de fin du test*/)
{
    if(strcmp(valeur,"0")==0){//Le test PCR est refusé
        return 0;
    }
    
    else if(strcmp(valeur,"1")==0){//Le test PCR est validé
        return 1;
    }
    
    else{ //Si la valeur n'est pas 0 ou 1
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

    dup2( argv0,0);        // Redirection de l'entrée sur le clavier
    dup2( argv1,1);        // Redirection de la sortie sur le terminal de la console

    ecritLigne(1,Message); //envoie le message

    char* reponse = litLigne(0); // Lit la réponse


    char emeteur[255], type[255], valeur[255];
    int msgDecoupe = decoupe(reponse, emeteur, type, valeur); //Découpe du message en 3 parties

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        printf("print : Erreur de découpage!!\n");
        exit(1);
    }

    if( analyseValeur(valeur)) { //Si la valeur est 1
        printf("Validé\n");
    }
    else{ //Si la valeur est 0
        printf("Refusé\n");
    }
   return 0;
}