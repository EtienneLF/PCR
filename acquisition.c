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

//Initialisation variables globales
int memoire;
int ** tab;
long long int * tabID;
char * local;

int df_Validation;
int df_Inter_Archives;

//Création sémaphore
sem_t s_memoire;
sem_t s_indice;

struct arg_st {
    int arg1;
    int arg2;
};

/**
 * Print l'erreur si le nombre d'argument passé est insuffisant
 * @param basename: argument
 **/
void usage(char * basename) { //print l'usage de la fonction
    fprintf(stderr,
        "usage : %s [<Taille mémoire>] [<Nom centre archivage>] [<Code de 4 chiffres>] [<Nom fichier résultats test PCR>] [Nombre terminal] [<Entrée Inter_Archives>] [<Sortie Inter_Archives>]\n",
        basename);
    exit(1);
}


/**
 * Retourne le descripteur de fichier correspondant dans la mémoire au numéro de test passé en paramètre
 * @param  numero : Numero du test
 **/
int thReponse(char* numero){
    for(int i = 0; i < sizeof(*tabID) ;i++){ //Pour chaque numéro de test PCR stocké dans la table de routage
        if(tabID[i] == strtoll(numero,NULL,10) && tab[0][i] == 1){ //Si le numéro dans la table de routage est égal au numéro passez en paramètre ET tab[0][i] == 1 (place prise dans la mémoire)
            int retour  = tab[1][i]; //Stock la valeur pour éviter d'être réécrite après la libération de la place
            tab[0][i] = 0; //Libère la place dans la table de routage
            return retour;
        }
    }
    fprintf(stderr, "Reponse n°%s, n'existe pas dans le stockage\n" , numero); //Erreur
    return -1;
}


/**
 * Retourne le numéro de l'indice du tableau contenant les descripteurs de ficiers
 **/
int indice(){
    for(int i = 0; i<memoire;i++){ //Pour chaque case regarde si tab[0][i] == 0, si oui retourne l'indice
        if( tab[0][i] == 0){
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
    tab[0][i] = 1; //Met à 1 pour verrouiller la place
    sem_post(&s_indice);//Sortie zone critique 
    tab[1][i] = olddf; //ajoute le descripteur de fichier dans la table de routage
    tabID[i] = strtoll(numero,NULL,10); //Ajoute le numéro du test PCR en long long int dans la table
}


/**
 * Retourne le descripteur de fichier vers lequel on veut envoyer le fichier depuis une Demande (Validation ou InterArchives)
 * Numero : Numero Pcr
 * oldff : Descirpteur de fichier
*/
int thDemande(char* numero, int olddf){
    //Retenir les 4 premiers numéro du test PCR
    char num[4];
    for (int i = 0; i<4; i++){
        num[i] = numero[i];
    }
    inserTab(numero,olddf); //ajoute dans la table de routage
    if(strcmp(num, local) == 0){ //Si les numéros du test correspond au serveur local, transmettre à Validation 
                                  //Sinon transmet à Inter_Archives
        fprintf(stderr, "Demande n°%s, transmie à Validation de %i\n" , numero,olddf);
        return df_Validation; //Retourne la sortie à utiliser
    }
    else{
        fprintf(stderr, "Demande n°%s, transmie à Inter_Archives de %i\n" , numero,olddf);
        return df_Inter_Archives;//Retourne la sortie à utiliser
        }
    return -1; //Si problème
}

/**
 * Fonction pour chaque nouveau Thread, Check le type de la demande (Demande ou Réponse) et envoie la demande vers la bonne destination
 * arg : msg, la demande de test PCR
 * df : descripteur de fichier
*/
void testNumero(char * msg, int fd){

    char numero[255], type[255], valeur[255];
    int msgDecoupe = decoupe(msg, numero, type, valeur); //Découpage du message
    int sortie;

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        fprintf(stderr, "Erreur de découpage!!\n");
        exit(1);
    }

    if(strcmp(type, "Demande") == 0){ //Si le message est une Demande, Attends une place de libre (sémaphore taille mémoire) puis appelle la fonction ThDemande
        sem_wait(&s_memoire);
        sortie = thDemande(numero,fd); //Retourne descripteur fichier de la sortie voulu
    }
    else if(strcmp(type, "Reponse") == 0){ //Si le message est une Réponse, Appelle la fonction ThReponse et Libère une place (sémaphore)
        sortie = thReponse(numero); //Retourne descripteur fichier de la sortie voulu
        fprintf(stderr, "Reponse n°%s, transmie à %d \n" , numero,sortie);
        sem_post(&s_memoire);
    }

    if(sortie == -1){ // Si erreur Open
        fprintf(stderr, "Sortie = -1 Erreur fonction testNumero Acquisition\n");
        exit(1);
    }

    if( ! ecritLigne(sortie,msg)){ // Si erreur écriture
        fprintf(stderr, "Erreur écriture \n");
        exit(1);
    }
    
}

void *th_function(void * args){
    /*fprintf(stderr,"%d\n", ((struct arg_st*)args) -> arg1);
    fprintf(stderr,"%d\n", ((struct arg_st*)args) -> arg2);*/
    int df = ((struct arg_st*)args) -> arg1;
    int fd = ((struct arg_st*)args) -> arg2;
    

    while(1){
        char * ligne = litLigne(df); //Lit la prochaine ligne  
        testNumero(ligne,fd);
    }
    pthread_exit(NULL); //Fin des threads
}

int main(int argc, char* argv[])
{ 
    if (argc != 8) usage(argv[0]); // Test nombre arguments
    memoire = atoi(argv[1]); //nombre place en mémoire
    char * name = argv[2]; //Nom du centre d'archivage
    local = argv[3]; //Numéro test local
    char * resulats_Pcr = argv[4]; //Nom fichier stockant les résultats
    int nrb_terminal = atoi(argv[5]); //nbr de terminal crée
    int I_A = atoi(argv[6]);
    df_Inter_Archives = atoi(argv[7]);

    fprintf(stderr,"Centre d'archivage numéro : %s et de nom : %s crée, Résultat sotcké dans : %s\n",local,name,resulats_Pcr);

    //Initialisation des sémaphores
    sem_init(&s_memoire,0,memoire);
    sem_init(&s_indice,0,1);

    //Initialisation Tableau Mémoire
    tab = calloc(2,sizeof(int)); //Tableau 2d qui stock en première ligne : 1 pour place occupée 0 pour place libre 
                                 //                        deuxième ligne : descripteur fichier
    for(int i = 0; i< 2;i++){
        tab[i] = calloc(memoire, sizeof(int));
    }
    tabID = calloc(memoire, sizeof(long long int)); //Tableau qui stock le numéro du test Pcr en long long int

    pthread_t v_thread_id;
    pthread_t i_thread_id;
    pthread_t t_thread_id[nrb_terminal];
    struct arg_st *args_I = (struct arg_st *)malloc(sizeof(struct arg_st));
    args_I -> arg1 = I_A;
    args_I -> arg2 = df_Inter_Archives;
    pthread_create(&i_thread_id, NULL, th_function, (void *)args_I);

    //Création procéssus validation + thread
    int pid;
    int A_V[2];
    pipe(A_V);
    int V_A[2];
    pipe(V_A);

    df_Validation = A_V[1];

    char * str_A_V = calloc(sizeof(char),2);
    sprintf(str_A_V, "%d", A_V[0]);
    char * str_V_A = calloc(sizeof(char),2);
    sprintf(str_V_A, "%d", V_A[1]);
    struct arg_st *args = (struct arg_st *)malloc(sizeof(struct arg_st));
    args -> arg1 = V_A[0];
    args -> arg2 = A_V[1];

    pid = fork();
        if (pid == 0){
            execlp("./validation", "./validation", str_A_V, str_V_A, resulats_Pcr,NULL);
            fprintf(stderr,"execlp() n'a pas fonctionné\n");
        }
    pthread_create(&v_thread_id, NULL, th_function, (void *)args);
    free(str_V_A);
    free(str_A_V);
    

    for(int i=0; i<nrb_terminal;i++){
        int A_T[2];
        pipe(A_T);
        int T_A[2];
        pipe(T_A);

        char * str_A_T = calloc(sizeof(char),2);
        sprintf(str_A_T, "%d", A_T[0]);

        char * str_T_A = calloc(sizeof(char),2);
        sprintf(str_T_A, "%d", T_A[1]);

        struct arg_st *args = (struct arg_st *)malloc(sizeof(struct arg_st));
        args -> arg1 = T_A[0];
        args -> arg2 = A_T[1];

        pid = fork();
        if (pid == 0){
            execlp("xterm", "xterm", "-e", "./terminal", str_A_T, str_T_A, NULL);
            fprintf(stderr,"execlp() n'a pas fonctionné\n");
        }
        free(str_T_A);
        free(str_A_T);
        pthread_create(&t_thread_id[i], NULL, th_function, (void *)args);
    }

    //Attentte de la fin des threads
    pthread_join(v_thread_id,NULL);
    //pthread_join(i_thread_id,NULL);
    for(int i=0;i<nrb_terminal;i++){ //Attends la fin des threads en fonction du compteur
        pthread_join(t_thread_id[i],NULL);
    }

    //libère la mémoire des deux tableaux
    free(tab);
    free(tabID);
    free(args);
   
   return 0;
}
