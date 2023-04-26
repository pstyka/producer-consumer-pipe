#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

int main(int argc, char* argv[])
{
    FILE * plik;

    char nazwa_pliku[200];
    int czytaj;
    char znak;

    sprintf(nazwa_pliku,"wy_%d",getpid());

    if(!(plik=fopen(nazwa_pliku,"w")))
    {
        perror("Blad otwarcia pliku producenta");
        exit(EXIT_FAILURE);
    }

    while((czytaj=read(0, &znak, sizeof(char)))>0)
    {
        if(czytaj==-1)
        {
            perror("Blad wczytywania znakow");
            exit(EXIT_FAILURE);
        }
        
        if(fwrite(&znak, sizeof(znak), 1, plik) == -1)
        {
            perror("Blad zapisu znaku do pliku");
            exit(EXIT_FAILURE);
        }
    }
    
    fclose(plik);
    exit(0);
}