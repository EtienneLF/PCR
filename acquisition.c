#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "lectureEcriture.h"
#include "message.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//fonction qui change la sortie en fonction de si c une demande ou une réponse

void usage(char * basename) { // DUPLICATION A CHANGER
    fprintf(stderr,
        "usage : %s [<Taille mémoire> ]\n",
        basename);
    exit(1);
}

/**
 * Retourne le descripteur de fichier vers lequel l'on veut envoyer le fichier en fonction du numero
 * Numero : Numero Pcr,
*/
int testNumero(char* numero, char* type){
    int fdValidation = open("Txt/E_validation.txt",O_WRONLY);
    int fd_IA = open("Txt/E_inter_archive.txt",O_WRONLY);
    int fdTerminal = open("Txt/E_terminal.txt",O_WRONLY);
    if(strcmp(numero, "0001") == 0){
        if (strcmp(type, "Demande") == 0){
            return fdValidation; // ok
        }
        else if(strcmp(type, "Reponse") == 0){
            return fdTerminal; // Tableau
        }
    }
    else{
        return fd_IA; //Pas bon
    }

    return -1;
}


/**
 * Retourne le descripteur de fichier vers lequel l'on veut envoyer le fichier
 * Numero : Numero Pcr, Type : Demande/Reponse
*/
/**
int redirectionSortie(char* numero, char* type){
    if(strcmp(type, "Demande") == 0 ){
        // Test les premiers numéros du test PCR pour Validation ou interarchives
        //int fdValidation = open("Txt/E_validation.txt",O_WRONLY);
        return testNumero(numero);
    }
    else if (strcmp(type, "Reponse") == 0){
        // Retrouver dans une liste numéro Pcr --> Descripteur fichier a retourner
        // Reponse soit terminal ou soit interarchive
        int fdTerminal = open("Txt/E_terminal.txt",O_WRONLY);
        return fdTerminal;
    }
    else{
        printf("Erreur, valeur non reconnue \n");
        return 1;
    }
}*/

int main(int argc, char* argv[])
{ 
    if (argc != 1) usage(argv[0]); // Test nombre arguments
    //char* memoire = argv[0];

    int fdTerminal = open("Txt/R_terminal.txt",O_RDONLY);
    char* ligne = litLigne(fdTerminal);
    
    while(strcmp(ligne, "erreur") != 0 ){
        char* msg = ligne; //Temp litLigne(0);

        //Faire un boucle qui lit tout les messages de R_qqch
        
        char numero[255], type[255], valeur[255];
        int msgDecoupe = decoupe(msg, numero, type, valeur);

        if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
            printf("print : Erreur de découpage!!\n");
            exit(1);
        }

        char num[5];
        for (int i = 0; i<4; i++){
            num[i] = numero[i];
        }

        int sortie = testNumero(num,type);
        if(sortie == -1){ // Si erreur Open
            printf("Erreur Open\n");
            exit(1);
        }
        
        dup2(sortie,1);
        if( ! ecritLigne(1,msg)){
            printf("Erreur écriture \n");
            exit(1);
        }
        ligne = litLigne(fdTerminal);
    }

    // |0001000000000000|Demande|13021|

    //dup2( argv0,0);        // Entrée Définit par nous au lancement
    //dup2( argv1,1);        // Sortie Changer en fonction de si réponse ou demande +  4 premier numéro
    // Doit garder une trace des numéros de test
   return 0;
}