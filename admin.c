#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define max_string_size 512
#define m 8
#define max_array_size 1024

struct request {
    char* filename;
    int* input_array;
    int input_length;
};

struct args {
    int* array;
    int array_offset;
    int array_size;
    // sem_t sem_array[];
};

void printer(int* array, int n) {
    int i;
    for (i = 0; i < n; i++) {
        printf("%d ", array[i]); 
    }
    printf("\n");
}

void *sorter(void* inputptr) {
    struct args* argin = (struct args*) inputptr;
    int i, j, tmp, min; int n = argin->array_size;
    int* array = (int*) ((argin->array) + argin->array_offset);
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

void *merger(void* inputptr) {
    struct args* argin = (struct args*) inputptr;
    int* array = (int*) ((argin->array) + argin->array_offset); int debug = 0;
    // if (array[0] != 16) return;
    int sz = argin->array_size;
    // printf("sz: %d\n", sz);
    int i = (sz/2)-1, j = i, n = sz, tmp;
    int ii = 0, jj = sz/2; int k;
    while (i >= 0 && j >= 0) {
        if (array[ii] <= array[jj]) {
            ii++;
            i--;
        }
        else {
            tmp = array[jj];
            for (k = jj; k>ii; k--) {
                array[k] = array[k-1];
            }
            array[ii] = tmp;
            jj++; ii++;
            j--;
        }
        if (debug) {
            printf("First element in thread is : %d and pointer is : %p\n", array[0], array);
            printf("Debug:\n");
            printer(array, sz);
            printf("ii is %d, jj is %d, [i,j]: %d %d\n\n", ii, jj, i, j);
        }
    }

}

struct request* receive() {
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

    char fname[50]; int* input_buffer; int input_length;
    recv(acceptance_fd, &fname, 50, 0);
    recv(acceptance_fd, &input_length, 100, 0);
    printf("Array Size is: %d\n", input_length);
    
    input_buffer = (int*) malloc(sizeof(int)*input_length);
    int tmp;
    for (i = 0; i < input_length; i++) {
        recv(acceptance_fd, &tmp, sizeof(int), 0);
        input_buffer[i] = tmp;
    }
    printf("Printing received data: \n");
    printer(input_buffer, input_length);
    printf("\n");
    close(soc);
    struct request* req = (struct request*) malloc(sizeof(struct request));
    req->filename = fname;
    req->input_array = input_buffer;
    req->input_length = input_length;

    return req;
}

int main() {
    
    int p, to_cal[2], from_cal[2];
    int array_size = 32;

    if (pipe(to_cal) == -1) {printf("Send pipe creation error !");};
    if (pipe(from_cal) == -1) {printf("Receive pipe creation error !");};

    p = fork();
    
    if (p == 0) {
        int i, tmp;
        int read_success = 0;
        int* array = malloc(array_size*sizeof(int));
        int sort_segment = array_size / m;
        int thread_count = m;
        pthread_t threads[thread_count];
        struct args *mergeparams;

        for (i=0; i<array_size;i++) {
            read(to_cal[0], &(array[i]), sizeof(int));
            write(from_cal[1], &read_success, sizeof(read_success));
        }
        int* arr = array;
        printf("Child received: \n");
        printer(array, 32);
        // return;
        int inc = 0;
        for ( i = 0; i < thread_count; i++ ){
            intptr_t* ptr = malloc(sizeof(intptr_t));
            *ptr = i;
            mergeparams = (struct args*) malloc(sizeof(struct args));
            mergeparams->array = arr;
            mergeparams->array_offset = inc;
            mergeparams->array_size = 4;
            pthread_create(&threads[i], NULL, &sorter, (void*)mergeparams);
            inc += 4;
        }

        for (i = 0; i < thread_count; i++) {
            pthread_join(threads[i], NULL);
        }
        printer(arr, 32);
        
        
        pthread_t threads_merge[4];
        inc = 0;
        for ( i = 0; i < 4; i++ ){
            intptr_t* ptr = malloc(sizeof(intptr_t));
            *ptr = i;
            mergeparams = (struct args*) malloc(sizeof(struct args));
            mergeparams->array = arr;
            mergeparams->array_offset = inc;
            mergeparams->array_size = 8;
            pthread_create(&threads_merge[i], NULL, &merger, (void*)mergeparams);
            inc += 8;
        }

        for (i = 0; i < 4; i++) {
            pthread_join(threads_merge[i], NULL);
        }
        printer(arr, 32);

        
        pthread_t threads_merge_two[2];
        inc = 0;
        for ( i = 0; i < 2; i++ ){
            intptr_t* ptr = malloc(sizeof(intptr_t));
            *ptr = i;
            mergeparams = (struct args*) malloc(sizeof(struct args));
            mergeparams->array = arr;
            mergeparams->array_offset = inc;
            mergeparams->array_size = 16;
            pthread_create(&threads_merge_two[i], NULL, &merger, (void*)mergeparams);
            inc += 16;
        }

        for (i = 0; i < 2; i++) {
            pthread_join(threads_merge_two[i], NULL);
        }
        printer(arr, 32);
        
        mergeparams = (struct args*) malloc(sizeof(struct args));
        mergeparams->array = arr;
        mergeparams->array_offset = 0;
        mergeparams->array_size = 32;
        merger(mergeparams);
        printer(arr, 32);
        

    }
    else {
        close(to_cal[0]);

        struct request* currentreq = receive();
        int* arr = currentreq->input_array;
        int i, read_status = -1;
        for (i=0; i < array_size; i++) {
            read_status = -1;
            write(to_cal[1], &(arr[i]), sizeof(int));
            read(from_cal[0], &read_status, sizeof(int));
            if (read_status == -1) {printf("Read failure from child !!");}
        }
        free(arr);
    }

    return 0;
}