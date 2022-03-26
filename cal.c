#include <stdio.h>
#include <string.h>

// adding merge sort function place holder here
// trying to understand what needs to be implemented
// at this point

void merge_sort(int* array) {

}

void main(int argc, char *argv[]) {
    // printf(" ==== Cal file started with file: %s !\n", argv[1]);
    int a, i,j; int k,l; int sign = 1;
    char  *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    char calc_type[20];
    char ch;
    ch = fgetc(fp);
    j = ch-'0';
    ch = fgetc(fp);

    // printf("%d", j);

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
        while (((ch = fgetc(fp)) != '\n') & ch != EOF) {
            if (ch == '-') {sign = -1;}
            else {
                l *= 10;
                l += ch - '0';        
            }
        }
        l *= sign; sign = 1;
        // printf("\nCalc type is %s, first arg is %d, second arg is %d\n", calc_type, k, l);
        
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

    // close the file
    fclose(fp);
    // printf("\nThe End!\n");
    return;

}