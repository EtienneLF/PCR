#include <stdio.h>
#include <string.h>
#include "message.h"
#include "alea.h"

char* createMsg(){
    char time[255];

    aleainit();// Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1,50000)); //durée aléatoire

    char *msg = message("0001000000000000", "Reponse", "1");

    return msg;
}

/**
 * Analyse de la réponse
 * 1 test valide
 * 0 test refusé
*/
void analyseReponse(char* valeur /*valeur de fin du test*/){
    //Le test PCR est refusé
    if(strcmp(valeur,"0")==0){
        printf("Refusé. \n");
    }
    //Le test PCR est validé
    else if(strcmp(valeur,"1")==0){
        printf("Validé. \n");
    }
    
    else{
        printf("Valeur non reconnue. \n");
    }
}

/**
 * Analyse du message
 * Demande
 * Réponse + anayse de la valeur 0 (refusé) ou 1(validé)
*/
void analyseMsg(char* msg /*message: n° test PCR, type, valeur*/){
    char emeteur[255], type[255], valeur[255];
    int msgDecoupe = decoupe(msg, emeteur, type, valeur);

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        printf("Erreur de découpage!!\n");
        return;
    }
    
    if(strcmp(type,"Demande")==0){//Demande
        printf("C'est une demande. \n");
    }
    else if (strcmp(type,"Reponse")==0){//Reponse
        printf("C'est une réponse.\n");
        analyseReponse(valeur);//Analyse 0(refusé) ou 1(validé)
    }
    else{
        printf("Erreur: type non reconnu.\n");
    }
    return;
}

int main(int argc, char *argv[])
{
    char * Message = createMsg();
    printf("Voici le message : %s", Message);
    analyseMsg(Message);


   return 0;
}