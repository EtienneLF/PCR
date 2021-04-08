#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "message.h"
#include "lectureEcriture.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void liaison(char* num){
    int df = open("centre_archivage.txt", O_WRONLY);
    int numPcr = atoi(num);
    if(strcmp(num,"0001")==0){
        fprintf(stderr, "oui");
        char* ligne = strcat(num, " Pcr1");
        fprintf(stderr,"non");
        ecritLigne(df,ligne); 

}

int analyse4Chiffres(char *pointeur, char* message){
    char numero[255], type[255], valeur[255];
    int msgDecoupe = decoupe(message, numero, type, valeur); 
    
    if(!msgDecoupe){ //Test le retour de la fonction découpe pour détecter une erreur
        fprintf(stderr, "Erreur de découpage!!\n");
        exit(1);
    }
    
    //char numPcr[4];
    for (int i=0; i<4; i++){
        pointeur[i] = numero[i];
    }

    //liaison(pointeur); //core dumped + stack smashing (?)
    return 0;
}



int main(int argc, char* argv[])
{
    char* test = message("0001000000000000", "Demande", "13021");
    //|0001000000000001|Demande|13021| 
    char num[4];
    analyse4Chiffres(num,test);
    liaison(num);
    fprintf(stderr,"Num : %s \n",num);
    return 0;
}