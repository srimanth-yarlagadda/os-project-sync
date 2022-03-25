#include <stdio.h>
#include <string.h>

#define max_string_size 512

int main() {
    int p; int k = getpid();
    int tunnel[2], feedback[2];

    if (pipe(tunnel) == -1) {
        printf("Pipe opening failure \n");
    };
    if (pipe(feedback) == -1) {
        printf("Pipe opening failure \n");
    };

    
    
    p = fork();

    if (p == 0) {
        int feedback_signal = 0, read_signal =0;
		char instruction_count_str[20], op_type[100], arg1_string[100], arg2_string[100];

        read(tunnel[0], instruction_count_str, 20);
        write(feedback[1], &feedback_signal, sizeof(feedback_signal));

        int instruction_count = atoi(instruction_count_str);
        int loop_iterator = 0;

		while(loop_iterator < instruction_count)
		{	
			
			read(tunnel[0], op_type, 100);
			write(feedback[1], &feedback_signal, sizeof(feedback_signal));
			
			read(tunnel[0], arg1_string, 100);
			write(feedback[1], &feedback_signal, sizeof(feedback_signal));

			read(tunnel[0], arg2_string, 100);
			loop_iterator++;

            char cmdstring[max_string_size];
            strcpy (cmdstring, "./cal-new.exe ");
            strcat (cmdstring, op_type);
            strcat (cmdstring, " ");
            strcat (cmdstring, arg1_string);
            strcat (cmdstring, " ");
            strcat (cmdstring, arg2_string);
            strcat (cmdstring, "\n");

			system(cmdstring);
			write(feedback[1], &feedback_signal, sizeof(feedback_signal));
		}
        
        }

    else {
        close(tunnel[0]);
        printf("Admin: %d %d\n", p, k);
        char  *filename2; int fni; int start = 0, check = 0;
 
        int a, i,j; int k,l; int sign = 1;
        char  *filename = "cal.in.txt";
        char *ch; 
        FILE *fp = fopen(filename, "r");
        char* calc_type[20];
        char command[max_string_size];
        fgets(command, max_string_size, fp);

        // printf("Printing the command value here %s\n", command);
        write(tunnel[1], command, 20);
        read(feedback[0], &check, sizeof(int));
        
        while(fgets(command, max_string_size, fp) != NULL)
		{
			i = 0; j = 0; char currsegment[20]; char a1[20]; char a2[20];
            
            char cp = command[i];
            while (cp != ' ') {                
                currsegment[i] = cp;
                i++;
                cp = command[i];
            }
            currsegment[i] = 0;
            i++;
            cp = command[i];

            int j = 0;
            while (cp != ' ') {                
                a1[j] = cp;
                i++; j++;
                cp = command[i];
            }
            a1[j] = 0;
            i++; j = 0;
            cp = command[i];

            while (cp != '\n') {                
                a2[j] = cp;
                i++; j++;
                cp = command[i];
            } 
            a2[j] = 0;

			write(tunnel[1], currsegment, 100);
			read(feedback[0], &check, sizeof(int));

			write(tunnel[1], a1, 100);
			read(feedback[0], &check, sizeof(int));

			write(tunnel[1], a2, 100);
			read(feedback[0], &check, sizeof(int));
		}

    }
        
    return 12;
}