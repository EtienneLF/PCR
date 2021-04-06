#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "lectureEcriture.h"
#include "message.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int memoire;
int ** tab;
long long int * tabID;


void usage(char * basename) {
    fprintf(stderr,
        "usage : %s [<Taille mémoire> ]\n",
        basename);
    exit(1);
}

int thReponse(char* numero){
    for(int i = 0; i < sizeof(*tabID) ;i++){
        if(tabID[i] == strtoll(numero,NULL,10) && tab[0][i] == 1){
            tab[0][i] = 0;
            return tab[1][i] ;
        }
    }
    fprintf(stderr, "Reponse n°%s, n'existe pas dans le stockage\n" , numero);
    return -1;
}

int indice(){
    for(int i = 0; i<memoire;i++){
        if( tab[0][i] == 0){
            return i;
        }
    }
    return -1;
}

void inserTab(char* numero, int olddf){
    int i = indice();

    tab[0][i] = 1;
    tab[1][i] = olddf;
    tabID[i] = strtoll(numero,NULL,10);
}

int thDemande(char* numero){
    //Ouverture des fichiers 
    int fdValidation = open("Txt/E_validation.txt",O_WRONLY);
    int fd_IA = open("Txt/E_inter_archive.txt",O_WRONLY);
    int olddf;

    //Retenir les 4 premiers numéro
    char num[4];
    for (int i = 0; i<4; i++){
        num[i] = numero[i];
    }

    olddf = dup(0);
    if(strcmp(num, "0001") == 0){
        fprintf(stderr, "Demande n°%s, transmie à Validation de %i\n" , numero,olddf);
        inserTab(numero,olddf); // a faire avant
        return fdValidation;
    }
    else{
        fprintf(stderr, "Demande n°%s, transmie à Inter_Archives de %i\n" , numero,olddf);
        inserTab(numero,olddf);
        return fd_IA;
        }
    return -1;
}

/**
 * Retourne le descripteur de fichier vers lequel l'on veut envoyer le fichier en fonction du numero et du type de demande
 * Numero : Numero Pcr, type : Demande ou Réponse
*/
void *testNumero(char * msg){
     
    char numero[255], type[255], valeur[255];
    int msgDecoupe = decoupe(msg, numero, type, valeur);
    int sortie;

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        fprintf(stderr, "Erreur de découpage!!\n");
        exit(1);
    }

    if(strcmp(type, "Demande") == 0){
        sortie = thDemande(numero);
    }
    else if(strcmp(type, "Reponse") == 0){
        sortie = thReponse(numero);
        fprintf(stderr, "Reponse n°%s, transmie à %d \n" , numero,sortie);
    }

    if(sortie == -1){ // Si erreur Open
        fprintf(stderr, "Erreur Open\n");
        exit(1);
    }

    dup2(sortie,1); // Redirige la sortie
    if( ! ecritLigne(1,msg)){ // Si erreur écriture
        fprintf(stderr, "Erreur écriture \n");
        exit(1);
    }

    return;
}

int main(int argc, char* argv[])
{ 

    if (argc != 1) usage(argv[0]); // Test nombre arguments
    //char* memoire = argv[0];

    memoire = 6;
    tab = calloc(2,sizeof(int));

    for(int i = 0; i< 2;i++){
        tab[i] = calloc(memoire, sizeof(int));
    }

    tabID = calloc(memoire, sizeof(long long int));


    int fdTerminal = open("Txt/R_inter_archive.txt",O_RDWR);
    dup2(fdTerminal,0);
    char* ligne = litLigne(fdTerminal);
    
    while(strcmp(ligne, "erreur") != 0 ){
        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, testNumero, ligne);
        testNumero(ligne);

        ligne = litLigne(fdTerminal);
    }

    
    pthread_join(thread_id,NULL);


    // Tableau a 3 lignes : 1 ère ligne = libre (0 ou 1 ), 2ème ligne = Id du test, 3ème ligne = descripteur fichier

    // |0001000000000000|Demande|13021|


    /*for(int i = 0; i< 2;i++){
        for(int j = 0; j< memoire;j++){
            fprintf(stderr, "%d ", tab[i][j]);
        }
        fprintf(stderr, "\n");
    }
    for(int i = 0; i< memoire;i++){
        fprintf(stderr, "%lli ", tabID[i]);
    }*/

    free(tab);
    free(tabID);
   
   return 0;
}