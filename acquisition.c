#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "lectureEcriture.h"
#include "message.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int memoire;
int ** tab;
long long int * tabID;
sem_t s_memoire;
sem_t s_ecriture;


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
void *testNumero(void * arg){
    char * msg = (char *) arg;

    char numero[255], type[255], valeur[255];
    int msgDecoupe = decoupe(msg, numero, type, valeur);
    int sortie;

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        fprintf(stderr, "Erreur de découpage!!\n");
        exit(1);
    }

    if(strcmp(type, "Demande") == 0){
        sem_wait(&s_memoire);
        sortie = thDemande(numero);
    }
    else if(strcmp(type, "Reponse") == 0){
        sortie = thReponse(numero);
        fprintf(stderr, "Reponse n°%s, transmie à %d \n" , numero,sortie);
        sem_post(&s_memoire);
    }

    if(sortie == -1){ // Si erreur Open
        fprintf(stderr, "Erreur Open\n");
        exit(1);
    }

    sem_wait(&s_ecriture);
    dup2(sortie,1); // Redirige la sortie

    if( ! ecritLigne(1,msg)){ // Si erreur écriture
        fprintf(stderr, "Erreur écriture \n");
        exit(1);
    }
    sem_post(&s_ecriture);
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{ 

    if (argc != 1) usage(argv[0]); // Test nombre arguments
    //char* memoire = argv[0];

    memoire = 6;
    sem_init(&s_memoire,0,memoire);
    sem_init(&s_ecriture,0,1);

    tab = calloc(2,sizeof(int));

    for(int i = 0; i< 2;i++){
        tab[i] = calloc(memoire, sizeof(int));
    }

    tabID = calloc(memoire, sizeof(long long int));


    int fdTerminal = open("Txt/R_inter_archive.txt",O_RDWR);
    dup2(fdTerminal,0);
    char* ligne = litLigne(fdTerminal);

    pthread_t thread_id[25]; 
    int nbr_th = 0;
    
    while(strcmp(ligne, "erreur") != 0 ){
        //Pour chaque Entrée lu : crée un nouveau Thread
        fprintf(stderr, "Lecture %s \n", ligne);
        pthread_create(&thread_id[nbr_th], NULL, testNumero, ligne);
        ligne = litLigne(fdTerminal);
        nbr_th += 1;
        
    }

    for(int i=0;i<nbr_th;i++){ //Attends la fin des threads
        pthread_join(thread_id[i],NULL);
    }


    //thread_join(thread_id,NULL);


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