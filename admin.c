#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define max_string_size 512
#define m 4 

void printer(int* array, int n) {
    int i;
    for (i = 0; i < n; i++) {
        printf("%d ", array[i]); 
    }
    printf("\n");
}

void *sorter(void* inputptr) {
    int i, j, tmp, min; int n = 4;
    int* array = (int*) inputptr;
    for (i=0; i<n-1; i++) {
        min = i;
        for (j=i+1; j<n; j++) {
            if (array[j] < array[min]) {
                min = j;
            }
        }
        if (min!=i) {
            tmp = array[i];
            array[i] = array[min];
            array[min] = tmp;
        }
    }
}

int* receive() {
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
        printf("Listening...\n\n");
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
    // int* arr = input_buffer;
    // qsort (input_buffer, sizeof(input_buffer)/sizeof(*input_buffer), sizeof(*input_buffer), comp);
    // sorter(input_buffer, input_length);
    printf("Printing received data: \n");
    printer(input_buffer, input_length);
    printf("\n");
    close(soc);
    return input_buffer;
}

int main() {
    int* arr;
    int p, to_cal[2], from_cal[2];
    arr = receive();
    char cmdstring[max_string_size];
    printf("Pointer is %p\n", arr);

    snprintf(cmdstring, sizeof(cmdstring), "./cal.exe %p", arr);
    // system(cmdstring);

    int array_size = 32;
    printf("Vals: %d %d\n", *arr, *(arr+4) ); 
    p = fork();
    
    if (p == 0) {
        int i;
        int* array = arr;
        int sort_segment = array_size / m;
        int thread_count = 8;
        pthread_t threads[thread_count];

        printer(arr, 32);
        int inc = 0;
        for ( i = 0; i < thread_count; i++ ){
            intptr_t* ptr = malloc(sizeof(intptr_t));
            *ptr = i;
            pthread_create(&threads[i], NULL, &sorter, (arr+inc));
            inc += 4;
        }

        for (i = 0; i < thread_count; i++) {
            pthread_join(threads[i], NULL);
        }

        printer(arr, 32);

        

    }
    else {
        // printf("Parent");
    }

    return 0;
}