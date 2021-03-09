#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>

#include <time.h>


int main(int argc, char* argv[])
{ 
    if (argc != 3) usage(argv[0]); // Test nombre arguments
    int argv0,argv1;
    sscanf (argv[0],"%d",&argv0); //conversion argv[0] en int
    sscanf (argv[1],"%d",&argv1); //conversion argv[1] en int

    dup2( argv0,0);        // Entrée     
    dup2( argv1,1);        // Sortie

    // Lire Entrée (donc reception Demande)
    // Séparé le message (avoir le numéro)
    // Lire si le nnuméro présent dans Pcr.txt
    // Vérifié validité
    // Ecrire la réponse sur la sortie

    
   return 0;
}