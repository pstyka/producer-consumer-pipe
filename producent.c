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
#include <string.h>

long str2Long(char* string);

int main(int argc, char* argv[])
{
    srand(time(NULL));

    FILE * plik;
    char nazwa_pliku[256];
    char znak;
    int i;
    char * end;
    int ilosc=str2Long(argv[1]);

    sprintf(nazwa_pliku,"we_%d",getpid());

    if(!(plik=fopen(nazwa_pliku,"w")))
    {
        perror("Blad otwarcia pliku producenta");
        exit(EXIT_FAILURE);
    }
    
    for(i=0;i<ilosc;i++)
    {
        znak = 'a' + rand() % 26;

        if(write(1, &znak, sizeof(char))==-1)
        {
            perror("Blad zapisywania znakow");
            exit(EXIT_FAILURE);
        }

       fprintf(plik, "%c", znak);
    }
    fclose(plik);
    sleep(2);
    exit(0);
}

long str2Long(char* string)
{
    char* end, str[100];
    long value;
    size_t last_item = strlen(string)-1;
    if(string[last_item]== '\n')
    {
        strncpy(str, string, last_item);
        value = strtol(str, &end, 10);
    }
    else
    {
        value = strtol(string, &end, 10);
    }
    if(errno == ERANGE)
    {
        perror("Wartosc przekracza zakres longa!!!!\n");
        exit(EXIT_FAILURE);
    }
    if(*end)
    {
        perror("Nie udalo sie skonwertowac ciagu znakow do long!!!!\n");
        exit(EXIT_FAILURE);
    }
    return value;

}