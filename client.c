#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void send_to_server(int* arrayToBeSent, int input_length) {
    
    // following code is taken from slides used in lecture
    int soc = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in servAddr;
    bzero ((char *) &servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr("10.176.92.14");
    servAddr.sin_port = htons(1038);

    int connect_status = connect (soc, (struct sockaddr *) &servAddr, sizeof(servAddr) );
    if (connect_status < 0) {
        printf("Connection Error !! \n");
    }
    else {
        printf("Connection Success !! \n");
    }

    char data[100] = "Hello World!!";
    if (send(soc, &input_length, 100, 0) < 0) {
        printf("Send Failure !!\n");
        return;
    }
    int i;
    for (i=0; i<input_length; i++) {
        send(soc, &(arrayToBeSent[i]), sizeof(int), 0);
        // printf("Sending %d: %d\n", i, arrayToBeSent[i]);
    }
    close(soc);
}

void main() {
    system("clear");
    printf("Read File Called\n");
    int i, l, input_length = 0; int sign = 1, insert = 0;
    char  *filename = "inptwo";
    FILE *fp = fopen(filename, "r");
    char ch;
    while ((ch = fgetc(fp)) != '\n') {
        input_length *= 10;
        input_length += ch - '0';
    }
    printf("Input length = %d\n", input_length);
    int* array_vals = (int*) malloc(sizeof(int)*input_length);
    i = 0;
    while (ch != EOF) {
        l = 0;
        while (((ch = fgetc(fp)) != '\n') & ch != EOF & ch != ' ') {
            insert = 1;
            if (ch == '-') {sign = -1;}
            else {
                l *= 10;
                l += ch - '0';        
            }
        }
        l *= sign; sign = 1;
        if (insert == 1) {
            (array_vals[i]) = l;
            i++;
        }
        insert = 0;
    }

    for (i = 0; i < input_length; i++) {
        printf("%d ", (array_vals)[i]); 
    }
    printf("\n");
    fclose(fp);
    // printf("Sending pointer: %p\n", array_vals);
    send_to_server(array_vals, input_length);
    printf(" *** Ending Client *** \n");
    return;
}