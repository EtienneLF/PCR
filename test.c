/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "lectureEcriture.h"
#include "message.h"
#include "alea.h"

#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_SIZE 80*/


/*char* createMsg(){
    char time[255];

    aleainit();// Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1,50000)); //durée aléatoire

    char *msg = message("0001000000000000", "Demande", time);

    return msg;
}

char* createMsg2(char* num){
    char time[255];

    aleainit();// Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1,50000)); //durée aléatoire

    char *msg = message(num, "Demande", time);

    return msg;
}


char* ligneAleatoire(int df, int num){
    char* lignePCR;

    for (int i =0; i<=num;i++){
        lignePCR = litLigne(df);
    }
    
    return lignePCR;
}*/

/*int main(int argc, char *argv[])
{*/
    /* char * Message = createMsg();
    printf("Voici le message : %s", Message);

    //int fd = open("README",O_RDONLY);
    //char* ligne = litLigne(0);

    ecritLigne(1,ligne);*/

    /* time_t timestamp = time( NULL );
    struct tm * pTime = localtime( & timestamp );

    char buffer[ MAX_SIZE ];
    strftime( buffer, MAX_SIZE, "%Y%m%d %H:%M:%S", pTime );
    printf( "Time : %s\n", buffer );

    printf("Seconds since January 1, 1970 = %ld\n", time(NULL));

    int tab[] ={1,2,3};

    for(int i =0; i<3; i++){
        printf(" oui : %d\n", tab[i]);
    }

    int a = tab[2];
    printf(" a : %d\n", a);
    tab[2] = 45;
    printf(" a : %d\n", a);

    for(int i =0; i<3; i++){
        printf(" oui : %d\n", tab[i]);
    } */ 


    /*int pid;

    for(int i = 0; i<2;i++)
    {
        pid = fork();
        if (pid != 0){
            //printf("Je suis le père\n");
        }
        else{
                execlp("xterm", "xterm", "-e", "./terminal", "0", "1",NULL);
                printf("execlp() n'a pas fonctionné\n");
            }
    }*/

    /**
     * CODE ALEATOIREEEEEE
     **/

    /*for (int i=0; i< 2; i++){//test création de 15 demandes
        int fd = open("Pcr2.txt",O_RDONLY);
        int aleaNum = alea(0,3); 
        char * ligne = ligneAleatoire(fd, aleaNum);
        fprintf(stderr, "Ligne n° %i: %s\n", aleaNum, ligne);   
        
        //Création du message
        char* emeteur = calloc(sizeof(char),16);
        
        for (int i =0; i < 16; i++){
            //fprintf(stderr, "Ligne de %i : %c \n", i,ligne[i]);   
            emeteur[i] = ligne[i];
        }
        fprintf(stderr, "Emeteur: %s \n",emeteur );
        char* msg = createMsg2(emeteur);
        fprintf(stderr, "Message crée: %s \n",msg );
        free(emeteur);
    }
    
    time_t seconds;
      
    seconds = time(NULL);
    printf("Seconds since January 1, 1970 = %ld\n", seconds);

    return 0;
} */

#include <stdio.h>
#include <string.h>
#include "message.h"
#include "alea.h"
#include "lectureEcriture.h"

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <time.h>

/**
 * Création du message dans le format |Numéro_du_test|Type|Valeur|
 **/
/*char* createMsg()
{
    char time[255];
    char num[255];

    aleainit(); // Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1615888497,1615988897)); //durée aléatoire

    sprintf(num,"%d",alea(1,9)); //durée aléatoire

    char numero[17]; 
    
    strcat(numero,"000100000000000");
    strcat(numero, num);

    char *msg = message(numero, "Demande", time); //Création d'une demande avec une durée de validité aléatoire

    return msg;
}*/

char* createMsg(char* num){
    char time[255];

    aleainit();// Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1,50000)); //durée aléatoire

    char *msg = message(num, "Demande", time);

    return msg;
}

char* ligneAleatoire(int df, int num){
    char* lignePCR;

    for (int i =0; i<=num;i++){
        lignePCR = litLigne(df);
    }
    
    return lignePCR;
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
        printf("Valeur non reconnue. \n");
        exit(1);
    }
}

int compteLigne(char * nomFichier){
    int df = open(nomFichier,O_RDONLY);
    char * ligne = litLigne(df);
    int temp = 0;
    while(strcmp(ligne, "erreur") != 0){
        temp +=1;
        ligne = litLigne(df);
    }
    
    return temp;
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
    //Demande
    if (argc != 4) usage(argv[0]); // Test nombre arguments
    int argv1,argv2;
    sscanf (argv[1],"%d",&argv1); //conversion argv[0] en int
    sscanf (argv[2],"%d",&argv2); //conversion argv[1] en int
    fprintf(stderr,"les deux sorties : %i , %i\n",argv1,argv2);

    int nbrLigne = compteLigne(argv[3]);
    fprintf(stderr, "Nombre de ligne: %d\n", nbrLigne);
    int fd = open(argv[3],O_RDONLY);
    int aleaNum = alea(0,nbrLigne); 
    char * ligne = ligneAleatoire(fd, aleaNum);
    fprintf(stderr, "Ligne n° %i: %s\n", aleaNum, ligne);   
    
    //Création du message
    char* num = calloc(sizeof(char),16);
    for (int i =0; i < 16; i++){
        num[i] = ligne[i];
    }

    fprintf(stderr, "Emeteur: %s \n",num );
    char* msg = createMsg(num);
    fprintf(stderr, "Message crée: %s \n",msg );
    free(num);
    fprintf(stderr," Voici le message : %s", msg); // Affichage du message
    
    

    dup2( argv1,0);        // Redirection de l'entrée
    dup2( argv2,1);        // Redirection de la sortie

    ecritLigne(1,msg); //envoie le message

    //Réponse
    char* reponse = litLigne(0); // Lit la réponse

    char emeteur[255], type[255], valeur[255];
    int msgDecoupe = decoupe(reponse, emeteur, type, valeur); //Découpe du message en 3 parties

    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        printf("print : Erreur de découpage!\n");
        exit(1);
    }
    if( analyseValeur(valeur)) { //Si la valeur est 1
        fprintf(stderr,"Validé\n");
    }
    else{ //Si la valeur est 0
        fprintf(stderr,"Refusé\n");
    }
    //while(1);

    
   return 0;
}