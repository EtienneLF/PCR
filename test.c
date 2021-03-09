#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "alea.h"
#include "message.h"
#include "lectureEcriture.h"
#include <time.h>

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
    char * Message = createMsg();
    printf("Voici le message : %s", Message);

    //nt fd = open("README",O_RDONLY);
    /*char* ligne = litLigne(0);

    ecritLigne(1,ligne);*/

    time_t timestamp = time( NULL );
    struct tm * pTime = localtime( & timestamp );

    char buffer[ MAX_SIZE ];
    strftime( buffer, MAX_SIZE, "%Y%m%d %H:%M:%S", pTime );
    printf( "Time : %s\n", buffer );

    printf("Seconds since January 1, 1970 = %ld\n", time(NULL));


    return 0;
} 