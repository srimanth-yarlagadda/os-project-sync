#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define max_string_size 512

int main() {    
   int i;

   // following code is taken from slides used in lecture
    int soc = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    bzero ((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(1038);
    printf("Socket in admin.c is: %d\n", soc);

    int bind_ret = bind(soc, (struct sockaddr *) &addr, sizeof(addr));
    if (bind_ret < 0) {printf("Error while binding\n");}
    else {printf("Binding Success !!\n");}
    
    int listen_status = listen(soc, 100);
    if (listen_status == 0) {
        printf("Listening !!\n");
    }
    else {
        printf("Listen Error !!\n");
    }
    
    int acceptance_fd = accept (soc, NULL, NULL);
    if (acceptance_fd < 0) {
        printf("Error Accepting!\n");
    }
    else {
        printf("Connection Accepted !!\n");
    }

    char data[100]; int* input_buffer; int input_length;
    recv(acceptance_fd, &input_length, 100, 0);
    printf("Array Size is: %d\n", input_length);
    
    input_buffer = (int*) malloc(sizeof(int)*input_length);
    int tmp;
    for (i = 0; i < input_length; i++) {
        recv(acceptance_fd, &tmp, sizeof(int), 0);
        input_buffer[i] = tmp;
    }
    printf("Printing received data: \n");
    for (i = 0; i < input_length; i++) {
        printf("%d ", input_buffer[i]); 
    }
    printf("\n");
    
    close(soc);    
    return 0;
}