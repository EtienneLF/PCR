#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "alea.h"
#include "message.h"
#include "lectureEcriture.h"
#include <time.h>
#include <unistd.h>

#define MAX_SIZE 80


char* createMsg(){
    char time[255];

    aleainit();// Creation d'une demande de validation pour une durée de validité aléatoire
    sprintf(time,"%d",alea(1,50000)); //durée aléatoire

    char *msg = message("0001000000000000", "Demande", time);

    return msg;
}





int main(int argc, char *argv[])
{
    /* char * Message = createMsg();
    printf("Voici le message : %s", Message);

    //nt fd = open("README",O_RDONLY);
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


    int pid;
    pid = fork();

    if (pid != 0){
        printf("Je suis le père\n");
    }
    else{
        execlp("./terminal", "0", "1",NULL);
        printf("execlp() n'a pas fonctionné\n");
    }

    return 0;
} 