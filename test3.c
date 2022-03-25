#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>


void handle_sigusr1(int signal)
{
	
}

void handle_sigusr2(int signal)
{

}

int main()
{     
	int p1[2], p2[2], x; // p1 is parent => child;; p2 is child => parent
	pipe(p1);
	pipe(p2);
	int pid = fork();
	if(pid == 0)
	{
		sleep(1);
		char str[60], str1[60], str2[60];
		int i = 0;
		struct sigaction sa_ch;
		sa_ch.sa_handler = &handle_sigusr2;
		sa_ch.sa_flags = SA_RESTART;

		sigaction(SIGUSR2, &sa_ch, NULL);
		pause();
		read(p1[0], str, 100);
		kill(getppid(), SIGUSR1);

		sigaction(SIGUSR2, &sa_ch, NULL);
		pause();
		read(p1[0], str1, 100);
		kill(getppid(), SIGUSR1);

		sigaction(SIGUSR2, &sa_ch, NULL);
		pause();
		read(p1[0], str2, 100);
		printf("The child has read: %s %s %s\n", str, str1, str2);
		kill(getppid(), SIGUSR1);
		
		
		
		
	}
	
	else
	{
		struct sigaction sa;
		sa.sa_handler = &handle_sigusr1;
		sa.sa_flags = SA_RESTART;
		

        	FILE* fp = fopen("cal2.in", "r");
		int i = 0;
		char str[60];
		char* token;
		char* a[3];
		fgets(str, 60, fp);
		while(fgets(str, 60, fp) != NULL)
		{
		token = strtok(str, " ");
		while(token != NULL)
		{
			token[strcspn(token, "\n")] = 0;
			a[i] = token;
			token = strtok(NULL, " ");
			i++;
		}
		sleep(1);
		printf("The parent is sending this string: %s\n", a[0]);
		write(p1[1], a[0], 100);
		kill(pid, SIGUSR2);

		sigaction(SIGUSR1, &sa, NULL);
		pause();
		printf("The parent is sending this string: %s\n", a[1]);
		write(p1[1], a[1], 100);
		kill(pid, SIGUSR2);

		sigaction(SIGUSR1, &sa, NULL);
		pause();
		printf("The parent is sending this string: %s\n", a[2]);
		write(p1[1], a[2], 100);
		kill(pid, SIGUSR2);

		sigaction(SIGUSR1, &sa, NULL);
		pause();
		}
		
		
		wait(NULL);

	}
	return 0;
}