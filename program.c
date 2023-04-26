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

long str2Long(char* str);

int main(int argc, char* argv[])
{
    int i;
    int fd[2]; // potok
    int prod, kons, ilosc, limit;
    int uruchomione_procesy;
    char bufor[256];
    char * end;
    FILE * sprawdz;
    prod = str2Long(argv[1]);
    kons = str2Long(argv[2]);
    ilosc = str2Long(argv[3]);
    
    //sprawdzenie ilosci parametrow wywolania
    if(argc != 4)
    {
        perror("Podano zla ilosc argumentow wywolania programu!!!\n");
        exit(EXIT_FAILURE);
    }

    //sprawdzenie ile procesow jest uruchomionych:
    sprawdz = popen("ps -ux | wc -l", "r");// funkcje pipe(), fork(), dup2() oraz exec().
    fgets(bufor, sizeof(bufor), sprawdz); // bufor,max,strumien na ktorym wykonujemy operacje
    pclose(sprawdz);
    uruchomione_procesy = str2Long(bufor);
    // -popen, -[PS], -[sh], -[wc-l]
    uruchomione_procesy -= 4;

    //dopuszczalny limit procesow
    printf("uruchomione procesy: %d\n",uruchomione_procesy);
    struct rlimit lim;
    if(getrlimit(RLIMIT_NPROC, &lim)==-1)
    {
        perror("Blad limitu\n");
        exit(EXIT_FAILURE);
    }
    else
        limit=lim.rlim_max;

    printf("Limit: %d\n",limit); // 257k
    
    if(kons < 1 || prod < 1 || ilosc < 1 ){
		printf("Argumenty musza byc dodanie\n!");
		exit(EXIT_FAILURE);
	}
    // Sprawdzenie, czy użytkownik może uruchomić odpowiednią liczbę procesów
    if(prod + kons + uruchomione_procesy > limit)
    {
        perror("Blad, nie moge utworzyc tak duzej ilosci procesow. Limit zostal przekroczony\n");
        exit(EXIT_FAILURE);
    }

    //tworzenie potoku
    if(pipe(fd) == -1)
    {
        perror("Blad tworzenia potoku\n");
        exit(EXIT_FAILURE);
    }
    //tworzenie procesow producent
    for(i=0;i<prod;i++)
    {
        switch(fork()){
				case -1:
					perror("fork error prod");
					if(errno==EAGAIN)
					{
					perror("W tej chwili pamiec nie jest dostepna\n");
					exit(EXIT_FAILURE);
}
// jesli zabraknie pamieci wyrzuci blad

					exit(EXIT_FAILURE);
				case 0:
					close(fd[0]); //zamykam odczyt
					if(dup2(fd[1],1)==-1) 
                    {
                        perror("dup error\n");
                        exit(EXIT_FAILURE);
                    }
                    close(fd[1]); // dup duplikuje  nie powielamy tablicy deskryptorow
					execlp("./producent","producent",argv[3],NULL);
					perror("execlp error producent");
					exit(EXIT_FAILURE);
				default:
                    break;
			}
    }    
    //tworzenie procesow konsument
    for(i=0;i<kons;i++)
    {
        switch(fork()){
				case -1:
					perror("fork error kons");
					if(errno==EAGAIN)
                                        {
                                        	perror("W tej chwili pamiec nie jest dostepna\n");
                                        	exit(EXIT_FAILURE);
					}
					exit(EXIT_FAILURE);
				case 0:
					close(fd[1]); // zamykanie deskryptora do zapisu
					if(dup2(fd[0],0)==-1)
                    {
                        perror("dup error\n");
                        exit(EXIT_FAILURE);
                    }
                    close(fd[0]);
					execlp("./konsument","konsument",NULL);
					perror("execlp error konsument");
					exit(EXIT_FAILURE);
				default:
                    break;
			}
    }


    if(close(fd[0])==-1)
    {
        perror("Blad zamykania deskryptora do odczytu\n");
        exit(EXIT_FAILURE);
    }
    if(close(fd[1])==-1)
    {
        perror("Blad zamykania deskryptora do zapisu\n");
        exit(EXIT_FAILURE);
    }
	int s;
	for(int i=0;i<prod + kons;i++)
	{
		wait(&s);
	}

    exit(0);
}

// producent fd[1] --------------------------> konsument fd[0]
long str2Long(char* string)
{
    char* end, str[100];
    long wartosc;
    size_t last_item = strlen(string)-1; // bo Wczytany wiersz jest zawsze zakończony znakiem terminalnym (\0).
    if(string[last_item]== '\n') // znak przejscia do nowej linii jest zapisywany do bufora 
    {
        strncpy(str, string, last_item);
        wartosc = strtol(str, &end, 10); //(string konwertowany, argument wyjsciowy, system liczbowy) 
    }
    else
    {
        wartosc = strtol(string, &end, 10);
    }
    if(errno == ERANGE) 
    {
        perror("Wartosc przekracza zakres longa!!!!\n");
        exit(EXIT_FAILURE);
    }
    if(*end)// jesli wyrzuci to oznacza to ze przekroczylismy zakres longa i nie udalo sie przekonwertowac calej liczby
    {
        perror("Nie udalo sie skonwertowac ciagu znakow do long!!!!\n");
        exit(EXIT_FAILURE);
    }
    return wartosc;
}
//potok nienazwany jest tworzony w pamieci
