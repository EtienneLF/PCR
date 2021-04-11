#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "message.h"
#include "lectureEcriture.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int ** tab_memoire;
int ** tab_df_acqui;
long long int * tabID;
int memoire;
int nbr_acqui;

//int tab_temp[2][3] = { {0001,0002,0003}  , {1,25,94} };

//Création sémaphore
sem_t s_memoire;
sem_t s_indice;

struct arg_st {
    int arg1;
    int arg2;
};

/**
 * Retourne le numéro de l'indice du tableau contenant les descripteurs de ficiers
 **/
int indice(){
    for(int i = 0; i<memoire;i++){ //Pour chaque case regarde si tab[0][i] == 0, si oui retourne l'indice
        if( tab_memoire[0][i] == 0){
            return i;
        }
    }
    return -1;
}

/**
 * Ajoute dans la table de routage le numéro du test Pcr et le descripteur de fichier pour le renvoyer
 * char * numero : Numero Pcr, int olddf : Duplicata du descripteur de l'entrée
*/
void inserTab(char* numero, int olddf){
    sem_wait(&s_indice); //Entrée zone critique 
    int i = indice(); //trouve le premier indice où tab[0][i] == 0
    tab_memoire[0][i] = 1; //Met à 1 pour verrouiller la place
    sem_post(&s_indice);//Sortie zone critique 
    tab_memoire[1][i] = olddf; //ajoute le descripteur de fichier dans la table de routage
    tabID[i] = strtoll(numero,NULL,10); //Ajoute le numéro du test PCR en long long int dans la table
}

int associationDemande(char* numero, int olddf){
    //Retenir les 4 premiers numéro du test PCR
    char num[4];
    for (int i = 0; i<4; i++){
        num[i] = numero[i];
    }
    inserTab(numero,olddf); //ajoute dans la table de routage

    //Associe num -> df dans tab_temp retourne dfd
    for (int i = 0; i<nbr_acqui; i ++){
        if (atoi(num) == tab_df_acqui[0][i]){
            fprintf(stderr, "Demande n°%s, transmie à Acquisition n°%i de %i\n" , numero,atoi(num),olddf);
            return tab_df_acqui[1][i];
        }
    }
    return -1; //Si problème
}

/**
 * Retourne le descripteur de fichier correspondant dans la mémoire au numéro de test passé en paramètre
 * Numero : Numero Pcr
*/
int associationReponse(char* numero){
    for(int i = 0; i < sizeof(*tabID) ;i++){ //Pour chaque numéro de test PCR stocké dans la table de routage
        if(tabID[i] == strtoll(numero,NULL,10) && tab_memoire[0][i] == 1){ //Si le numéro dans la table de routage est égal au numéro passez en paramètre ET tab[0][i] == 1 (place prise dans la mémoire)
            int retour  = tab_memoire[1][i]; //Stock la valeur pour éviter d'être réécrite après la libération de la place
            tab_memoire[0][i] = 0; //Libère la place dans la table de routage
            return retour;
        }
    }
    fprintf(stderr, "Reponse n°%s, n'existe pas dans le stockage\n" , numero); //Erreur
    return -1;
}

void testType(char * msg, int fd){

    char numero[255], type[255], valeur[255];
    int msgDecoupe = decoupe(msg, numero, type, valeur); //Découpage du message
    int sortie ;

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        fprintf(stderr, "Erreur de découpage!!\n");
        exit(1);
    }

    if(strcmp(type, "Demande") == 0){ //Si le message est une Demande, Attends une place de libre (sémaphore taille mémoire) puis appelle la fonction ThDemande
        sem_wait(&s_memoire);
        sortie = associationDemande(numero,fd); //Retourne descripteur fichier de la sortie voulu
    }
    else if(strcmp(type, "Reponse") == 0){ //Si le message est une Réponse, Appelle la fonction ThReponse et Libère une place (sémaphore)
        sortie = associationReponse(numero); //Retourne descripteur fichier de la sortie voulu
        fprintf(stderr, "Reponse n°%s, transmie à %d \n" , numero,sortie);
        sem_post(&s_memoire);
    }

    if(sortie == -1){ // Si erreur Open
        fprintf(stderr, "Sortie = -1 Erreur fonction testType InterArchives\n");
        exit(1);
    }

    if( ! ecritLigne(sortie,msg)){ // Si erreur écriture
        fprintf(stderr, "Erreur écriture \n");
        exit(1);
    }
    
}

void *th_function(void * args){
    int df = ((struct arg_st*)args) -> arg1;
    int fd = ((struct arg_st*)args) -> arg2;

    while(1){
        char * ligne = litLigne(df); //Lit la prochaine ligne  
        testType(ligne,fd);
    }
    pthread_exit(NULL); //Fin des threads
}

int main(int argc, char* argv[])
{    
    char * memoirestr = "6";
    memoire = 6; 
    //nbr_acqui = 3;
    char * name = "Pcr.txt";
    char * nbr_terminal = "2";

    //Initialisation des sémaphores
    sem_init(&s_memoire,0,memoire);
    sem_init(&s_indice,0,1);    
    
    tab_memoire = calloc(2,sizeof(int)); //Tableau 2d qui stock en première ligne : 11 pour place occupée 0 pour place libre 
                                        //                        deuxième ligne : descripteur fichier
    for(int i = 0; i< 2;i++){
        tab_memoire[i] = calloc(memoire, sizeof(int));
    }


    tabID = calloc(sizeof(long long int),memoire); //Tableau qui stock le numéro du test Pcr en long long int

    //|0001000000000001|Demande|13021| 
    //int id = recuperation4Chiffres("|0001000000000001|Demande|13021|");
    //testType("|0001000000000001|Demande|13021|",0);
    //testType("|0001000000000001|Reponse|1|",0);
    //fprintf(stderr, "Voici les 4 premiers chiffres: %d\n",atoi(num));

    int df = open("centre_archivage.txt", O_RDONLY);
    char* ligne = litLigne(df);
    nbr_acqui = 0;
    while(strcmp(ligne, "erreur") != 0){
        nbr_acqui++;
        ligne = litLigne(df);
    }
    close(df);
    pthread_t thread_id[nbr_acqui];

    tab_df_acqui = calloc(2,sizeof(int)); //Tableau 2d qui stock en première ligne : Numéro du centre déarchivage
                                          //                        deuxième ligne : descripteur fichier lié
    for(int i = 0; i< 2;i++){
        tab_df_acqui[i] = calloc(nbr_acqui, sizeof(int));
    }

    df = open("centre_archivage.txt", O_RDONLY);
    char * num = calloc(sizeof(char),4);

    for(int i = 0; i<nbr_acqui; i++){
        ligne = litLigne(df);
        for (int i =0; i<4; i++){ //lire les 4 num du fichier
            num[i] = ligne[i];
        }

        char* centre =  calloc(sizeof(char),strlen(ligne)-6);
        for (int i = 0; i< strlen(ligne)-1; i++){
            centre[i] = ligne[i+5];
        }        

        int I_A[2];
        pipe(I_A);
        int A_I[2];
        pipe(A_I);

        tab_df_acqui[0][i] = atoi(num);
        tab_df_acqui[1][i] = I_A[1]; //Stock dans le tableau les futurs valeurs

        char * str_I_A = calloc(sizeof(char),2);
        sprintf(str_I_A, "%d", I_A[0]);

        char * str_A_I = calloc(sizeof(char),2);
        sprintf(str_A_I, "%d", A_I[1]);

        struct arg_st *args = (struct arg_st *)malloc(sizeof(struct arg_st));
        args -> arg1 = A_I[0];
        args -> arg2 = I_A[1];
        
        fprintf(stderr,"str_A_T : %i , %i str_T_A : %i , %i\n",I_A[0],I_A[1],A_I[0],A_I[1]);

        int pid = fork();
        if (pid == 0){
            execlp("xterm", "xterm", "-e", "./acquisition", memoirestr, centre, num, name, nbr_terminal,str_I_A, str_A_I,NULL);
            fprintf(stderr,"execlp() n'a pas fonctionné\n");
        }
        free(str_I_A);
        free(str_A_I);
        pthread_create(&thread_id[i], NULL, th_function, (void *)args);

        fprintf(stderr,"Numéro %s associé au centre %s\n", num, centre);
    }

    for(int i=0;i<nbr_acqui;i++){ //Attends la fin des threads en fonction du compteur
        pthread_join(thread_id[i],NULL);
    }

    free(tab_memoire);
    free(tabID);
    free(tab_df_acqui);
    return 0;
}