#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "lectureEcriture.h"
#include "message.h"

#include <search.h>

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
 * Retourne le descripteur de fichier vers lequel l'on veut envoyer le fichier en fonction du numero et du type de demande
 * Numero : Numero Pcr, type : Demande ou Réponse
*/
int testNumero(char* numero, char* type, int** tab, long long int* tabID){
    //Ouverture des fichiers 
    int fdValidation = open("Txt/E_validation.txt",O_WRONLY);
    int fd_IA = open("Txt/E_inter_archive.txt",O_WRONLY);
    int fdTerminal = open("Txt/E_terminal.txt",O_WRONLY);
    int olddf;

    //Retenir les 4 premiers numéro
    char num[5];
    for (int i = 0; i<4; i++){
        num[i] = numero[i];
    }

    if(strcmp(type, "Demande") == 0){
        olddf = dup(0);
        if(strcmp(num, "0001") == 0){
            fprintf(stderr, "Demande n°%s, transmie à Validation de %i\n" , numero,olddf);
            tab[0][0] = 1;
            tab[1][0] = olddf;
            tabID[0] = strtoll(numero,NULL,10);
            return fdValidation;
        }
        else{
            fprintf(stderr, "Demande n°%s, transmie à Inter_Archives de %i\n" , numero,olddf);
            tab[0][1] = 1;
            tab[1][1] = olddf;
            tabID[1] = strtoll(numero,NULL,10);
            return fd_IA;
        }
    }
    else if(strcmp(type, "Reponse") == 0){
        fprintf(stderr, "Reponse n°%s, transmie à Terminal (temp)\n" , numero);
        return fdTerminal;//Tableau
    }
    
    return -1;
    
}

int main(int argc, char* argv[])
{ 

    if (argc != 1) usage(argv[0]); // Test nombre arguments
    //char* memoire = argv[0];

    int memoire = 6;

    int ** tab = calloc(2,sizeof(int));

    for(int i = 0; i< 2;i++){
        tab[i] = calloc(memoire, sizeof(int));
    }


    long long int * tabID = calloc(memoire, sizeof(long long int));


    int fdTerminal = open("Txt/R_inter_archive.txt",O_RDONLY);
    char* ligne = litLigne(fdTerminal);
    
    while(strcmp(ligne, "erreur") != 0 ){

        char* msg = ligne; //Temp litLigne(0);
 
        char numero[255], type[255], valeur[255];
        int msgDecoupe = decoupe(msg, numero, type, valeur);

        if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
            fprintf(stderr, "Erreur de découpage!!\n");
            exit(1);
        }

        int sortie = testNumero(numero,type,tab,tabID);
        if(sortie == -1){ // Si erreur Open
            fprintf(stderr, "Erreur Open\n");
            exit(1);
        }
        
        dup2(sortie,1);
        if( ! ecritLigne(1,msg)){ // Si erreur écriture
            fprintf(stderr, "Erreur écriture \n");
            exit(1);
        }
        ligne = litLigne(fdTerminal);
    }


    // Tableau a 3 lignes : 1 ère ligne = libre (0 ou 1 ), 2ème ligne = Id du test, 3ème ligne = descripteur fichier



    // |0001000000000000|Demande|13021|

    //dup2( argv0,0);        // Entrée Définit par nous au lancement
    //dup2( argv1,1);        // Sortie Changer en fonction de si réponse ou demande +  4 premier numéro
    // Doit garder une trace des numéros de test
    
    
    
    /*

    ENTRY e,*ep;
    hcreate(3); //Crée une table de x élement 

    e.key = "0"; // Crée une clée
    e.data = (void *) 654654; // La valeur stocké avec la clé
    ep = hsearch(e,ENTER); // Ajoute e dans la hasmap
    if (ep == NULL) { //Si erreur
        fprintf(stderr, "Échec\n");
        exit(EXIT_FAILURE);
    }

    e.key = "0"; // Clé pour search
    ep = hsearch(e,FIND);
    fprintf(stderr, "%9.9s -> %9.9s:%d\n", e.key, ep ? ep->key : "NULL", ep ? (int)(ep->data) : 0);


    destroy(); // Libère la table de la mémoire


    */
   
  
    for(int i = 0; i< 2;i++){
        for(int j = 0; j< memoire;j++){
            fprintf(stderr, "%d ", tab[i][j]);
        }
        fprintf(stderr, "\n");
    }

    for(int i = 0; i< memoire;i++){
        fprintf(stderr, "%lli ", tabID[i]);
    }

    free(tab);
    free(tabID);
   
   return 0;
}