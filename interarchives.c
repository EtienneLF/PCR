#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include "message.h"
#include "lectureEcriture.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Initialisation variables globales
int ** tab_memoire;             //Partie 1 de la table de routage
int ** tab_df_acqui;            //Mémoire de la table des descripteurs fichiers
long long int * tabID;          //Partie 2 de la table de routage
int memoire;                    //Taille de la mémoire de la table de routage
int nbr_acqui;                  //Nombre serveur acquisition

//Création sémaphore
sem_t s_memoire;
sem_t s_indice;

//Structure pour arguments Thread
struct arg_st {
    int arg1;
    int arg2;
};

/**
 * Decoupe du message 
 * Retourne 1 si il n'y a pas eu de problème
 * Retourne 0 si il y a eu un problème - errno est mis � EINVAL dans ce cas.
 * @param message: Source a découper
 * @param nServ: n° du Serveur PCR concerne
 * @param nbr_t: Nombre Terminal
 * @param nom_Fichier: Nom fichier
 * @param nom_Serveur: Nom du serveur
 * ATTENTION:
 *     Les variables nServ, nbr_t, nom_Fichier et nom_Serveur doivent 
 * être allouées avant l'appel à decoupe
 */
int decoupe2(char *message, char *nServ, char *nbr_t, char *nom_Fichier, char *nom_Serveur)
{
  int nb=sscanf(message, "%s %s %s %s\n", nServ, nbr_t, nom_Fichier,nom_Serveur);
  if (nb == 4)
    return 1;
  else  {
    errno = EINVAL;
    return 0;
  }
}

/**
 * Print l'erreur si le nombre d'argument passé est insuffisant
 * @param basename: argument
 **/
void usage(char * basename) { //print l'usage de la fonction
    fprintf(stderr,
        "usage : %s [<Taille mémoire>] [<Nom fichier config>]\n",
        basename);
    exit(1);
}

/**
 * Retourne le numéro de l'indice du tableau contenant les descripteurs de fichiers
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
 * @param numero : Numero Pcr
 * @param olddf : Duplicata du descripteur de l'entrée
*/
void inserTab(char* numero, int olddf){
    sem_wait(&s_indice); //Entrée zone critique 
    int i = indice(); //trouve le premier indice où tab[0][i] == 0
    tab_memoire[0][i] = 1; //Met à 1 pour verrouiller la place
    sem_post(&s_indice);//Sortie zone critique 
    tab_memoire[1][i] = olddf; //ajoute le descripteur de fichier dans la table de routage
    tabID[i] = strtoll(numero,NULL,10); //Ajoute le numéro du test PCR en long long int dans la table
}


/**
 *  Associe les demandes au descripteur de fichier correspondant
 * @param numero: numéro du test
 * @param olddf: descripteur de fichier que l'on doit rajouter ds la table de routage
 **/
int associationDemande(char* numero, int olddf){
    //Retenir les 4 premiers numéro du test PCR
    char num[4];
    for (int i = 0; i<4; i++){
        num[i] = numero[i];
    }
    inserTab(numero,olddf); //ajoute dans la table de routage

    //Associe num -> df dans tab_temp retourne dfd
    for (int i = 0; i<nbr_acqui; i ++){
        if (atoi(num) == tab_df_acqui[0][i]){ //si les 4 premiers chiffres du test correspondent à ceux stocker ds le tableau
            fprintf(stderr, "Demande n°%s, transmie à Acquisition n°%i de %i\n" , numero,atoi(num),olddf);
            return tab_df_acqui[1][i];
        }
    }
    return -1; //Si problème
}

/**
 * Retourne le descripteur de fichier correspondant dans la mémoire au numéro de test passé en paramètre
 * @param numero : Numero Pcr
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


/**
 * Analyse le type de la requête
 * @param msg: Message
 * @param fd: descripteur de fichier
 **/
void testType(char * msg, int fd){

    char numero[255], type[255], valeur[255];
    int msgDecoupe = decoupe(msg, numero, type, valeur); //Découpage du message
    int sortie;

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

/**
 * Fonction exécuté par le Thread
 * @param args: Structure contenant les deux arguments de la fonction, deux descripteurs de fichiers
 **/
void *th_function(void * args){
    int df = ((struct arg_st*)args) -> arg1; //Entrée
    int fd = ((struct arg_st*)args) -> arg2; //SOrtie

    while(1){
        char * ligne = litLigne(df); //Lit la prochaine ligne  
        testType(ligne,fd); //analyse de la ligne
    }
    pthread_exit(NULL); //Fin des threads
}

int main(int argc, char* argv[])
{    
    if (argc != 3) usage(argv[0]); //Test nombre arguments
    char * memoirestr = argv[1]; //Nombre de place dans la mémoire
    memoire = atoi(memoirestr); //Transforme le char* en int
    char * config = argv[2];

    //Initialisation des sémaphores
    sem_init(&s_memoire,0,memoire);
    sem_init(&s_indice,0,1);    
    
    tab_memoire = calloc(2,sizeof(int)); //Tableau 2d qui stock en première ligne : 1 pour place occupée 0 pour place libre 
                                         //                        deuxième ligne : descripteur fichier
    for(int i = 0; i< 2;i++){ //Initialisation de calloc dans le tableau
        tab_memoire[i] = calloc(memoire, sizeof(int));
    }

    tabID = calloc(sizeof(long long int),memoire); //Tableau qui stock le numéro du test Pcr en long long int

    //Compte le nombre de ligne du fichier pour la suite (Thread_id par exemple)
    int df = open(config, O_RDONLY); //ouvre le Fichier
    char* ligne = litLigne(df); //Lit la première ligne
    nbr_acqui = 0; //Compteur
    while(strcmp(ligne, "erreur") != 0){ //Tant que la dernière ligne n'est pas vide
        nbr_acqui++; //Incrément le compteur
        ligne = litLigne(df);
    }
    close(df);
    //Création des adresse ou seront stockés les threads Id
    pthread_t thread_id[nbr_acqui]; //Tableau d'adresse en fonction du nombre de ligne (nombre de serveur)

    tab_df_acqui = calloc(2,sizeof(int)); //Tableau 2d qui stock en première ligne : Numéro du centre déarchivage
                                          //                        deuxième ligne : descripteur fichier lié
    for(int i = 0; i< 2;i++){//Initialisation de calloc dans le tableau
        tab_df_acqui[i] = calloc(nbr_acqui, sizeof(int));
    }

    df = open(config, O_RDONLY); //Réouvre le fichier

    for(int i = 0; i<nbr_acqui; i++){ //Pour chaque ligne dans le fichier
        ligne = litLigne(df); //Lit une ligne

        //Découpe de la ligne
        char num[255], nbr_t[255], nom_fichier[255], nom_Serveur[255];
        int msgDecoupe = decoupe2(ligne,num,nbr_t,nom_fichier,nom_Serveur);    
        if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
            fprintf(stderr, "Erreur de découpage!!\n");
            exit(1);
        }

        //Création d'une paire de tube pour la communication avec le serveur acquisition
        int I_A[2];
        pipe(I_A);
        int A_I[2];
        pipe(A_I);

        //Stock dans le tableau le numéro du serveur et son descripteur de fichier associé
        tab_df_acqui[0][i] = atoi(num);
        tab_df_acqui[1][i] = I_A[1];

        //Convertit les descripteur de fichiers en cahr * pour les passés en arguments de execlp
        char * str_I_A = calloc(sizeof(char),2);
        sprintf(str_I_A, "%d", I_A[0]);

        char * str_A_I = calloc(sizeof(char),2);
        sprintf(str_A_I, "%d", A_I[1]);

        struct arg_st *args = (struct arg_st *)malloc(sizeof(struct arg_st)); //Arguments
        args -> arg1 = A_I[0];
        args -> arg2 = I_A[1];

        int pid = fork(); //Création d'un processus
        if (pid == 0){ //Si Fils
            execlp("xterm", "xterm", "-e", "./acquisition", memoirestr, nom_Serveur, num, nom_fichier, nbr_t,str_I_A, str_A_I,NULL); //Crée un nouveau terminal avec acquisition
            fprintf(stderr,"execlp() n'a pas fonctionné Inter_Archives\n"); //Erreur
        }
        //Libère les calocs
        free(str_I_A);
        free(str_A_I);
        pthread_create(&thread_id[i], NULL, th_function, (void *)args); //Création du Thread
    }

    for(int i=0;i<nbr_acqui;i++){ //Attends la fin des threads en fonction du compteur
        pthread_join(thread_id[i],NULL);
    }

    //Libère la mémoire
    free(tab_memoire);
    free(tabID);
    free(tab_df_acqui);
    return 0;
}