#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct file_details {
    char* filename;
    int* input_array;
    int input_length;
};

struct file_details* read_request(char* filename) {
    printf("Read request...\n");
    int i, l, input_length = 0; int sign = 1, insert = 0;
    FILE *fp = fopen(filename, "r");
    char ch;
    while ((ch = fgetc(fp)) != '\n') {
        input_length *= 10;
        input_length += ch - '0';
    }
    printf("[%d] values: ", input_length);
    struct file_details* file_info = (struct file_details*) malloc(sizeof(struct file_details));
    int* input_array = (int*) malloc(sizeof(int)*input_length);
    
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
            (input_array[i]) = l;
            i++;
        }
        insert = 0;
    }

    for (i = 0; i < input_length; i++) {
        printf("%d ", (input_array)[i]); 
    }
    printf("\n");
    fclose(fp);
    
    file_info->filename = filename;
    file_info->input_array = input_array;
    file_info->input_length = input_length;
    return file_info;
}

void send_to_server(char* fname, int* arrayToBeSent, int input_length) {
    char fnname[200];
    strcat(fnname, fname);
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
        // printf("Connection Success !! \n");
    }
    
    // printf("client print %s\n", fnname);
    send(soc, fnname, sizeof(fnname), 0);
    send(soc, &input_length, sizeof(int), 0);
    // if (send(soc, &fnname, 200, 0) < 0) {
    //     printf("Send Failure !!\n");
    //     return;
    // }
    // if (send(soc, &input_length, 100, 0) < 0) {
    //     printf("Send Failure !!\n");
    //     return;
    // }
    int i;
    for (i=0; i<input_length; i++) {
        send(soc, &(arrayToBeSent[i]), sizeof(int), 0);
        // printf("Sending %d: %d\n", i, arrayToBeSent[i]);
    }
    printf("Request %s sent to server...\n\n", fnname);
    close(soc);
}

void main() {
    system("clear");
    int clientid, run = 500;
    
    char scanfilename[200];
    // char ip_address; memset(ip_address, 0, 50);
    // strcpy(ip_address, "10.176.92.14");

    // char readip[200]; char def[200];
    // strcpy(def, "default");
    // printf("Enter server IP (\"default\" for 10.176.92.14): ");
    // scanf("%s", readip);

    // if (strcmp(readip, def) != 0) {
        // ip_address = readip;
    // }

    while(run > 0) {
        printf("Enter filename: ");
        scanf("%s", scanfilename);
        
        // scanfilename = "i";
        if (strcmp(scanfilename, "end") == 0) break;
        struct file_details* request = read_request(scanfilename);
        send_to_server(request->filename, request->input_array, request->input_length);
        free(request);
        run--;
        memset(scanfilename, 0, 200);
    }

    /*
        request = read_request(scanfilename);
        send_to_server(request->filename, request->input_array, request->input_length);
        free(request);
        run--;
        scanfilename[0] -= 1;
    

        request = read_request(scanfilename);
        send_to_server(request->filename, request->input_array, request->input_length);
        free(request);
        run--;
        scanfilename[0] += 2;
    
        
    }
    */
    printf(" *** Ending Client *** \n");
    return;
}