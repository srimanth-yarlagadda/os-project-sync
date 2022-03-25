#include <stdio.h>
#include <string.h>

#define max_string_size 512
// extern int tunnel[2];
// extern pipe tunnel;

void calnew(char s[], char s1[], char s2[]) {
    
    // printf(" == [NEW] Cal file started ! ==\n");
    // int a, i,j; int sign = 1;
    char* calc_type = s;
    int k = atoi(s1);
    int l = atoi(s2);

    // printf("type : %s, arg 1: %d, arg2 : %d", argv[1],k,l);


    // close(tunnel[1]);
    // char *fname; int fn;
    // if (read(tunnel[0], &fname, max_string_size) == -1) {
    //     printf("Bad read !! \n");
    //     return 10;
    // };
    // close(tunnel[0]);
    // printf("child process read %s\n", &fname);

    if (calc_type[0] == 'q') {
        printf("first\n");
        solve_quadratic(k,l);
    }
    else if (calc_type[0] == 't') {
        printf("second\n");
        find_triples(k,l);
    }
    else if (calc_type[0] == 'm') {
        printf("third\n");
        modular_sum(k,l);
    }



    // char  *filename = argv[1];

    /*
    FILE *fp = fopen(filename, "r");
    char calc_type[20];
    char ch;
    ch = fgetc(fp);
    j = ch-'0';
    ch = fgetc(fp);

    printf("%d", j);

    for (a=0; a<j; a++) {
        i = 0;
        while ((ch = fgetc(fp)) != ' ') {
            calc_type[i] = ch;
            i++;
        }
        calc_type[i] = 0;
    
        k = 0;
        while ((ch = fgetc(fp)) != ' ') {
            if (ch == '-') {sign = -1;}
            else {
                k *= 10;
                k += ch - '0';
            }
        }
        k *= sign; sign = 1;

        l = 0;
        while ((ch = fgetc(fp)) != '\n') {
            if (ch == '-') {sign = -1;}
            else {
                l *= 10;
                l += ch - '0';        
            }
        }
        l *= sign; sign = 1;
        printf("\nCalc type is %s, first arg is %d, second arg is %d\n", calc_type, k, l);
        
        if (calc_type[0] == 'q') {
            printf("first\n");
            solve_quadratic(k,l);
        }
        else if (calc_type[0] == 't') {
            printf("second\n");
            find_triples(k,l);
        }
        else if (calc_type[0] == 'm') {
            printf("third\n");
            modular_sum(k,l);
        }
    }
    fclose(fp);
    */
    
    
    
    return;

}