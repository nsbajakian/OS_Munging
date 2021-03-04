/**
 * @file m64.c
 * @author Nathaniel Bajakian (nsbajakian@wpi.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sched.h>

#define KRED  "\x1B[31m"
#define ONE_MEGABYTE 10000000L

void Munge64( void *data, uint64_t size ) {
    double *data64 = (double*) data;
    double *data64End = data64 + (size >> 3); /* Divide size by 8. */
    uint8_t *data8 = (uint8_t*) data64End;
    uint8_t *data8End = data8 + (size & 0x00000007); /* Strip upper 29 bits. */
    
    while( data64 != data64End ) {
        *data64++ = -*data64;
    }
    while( data8 != data8End ) {
        *data8++ = -*data8;
    }
}

int main(int argc, char** argv) {

    if(argc != 3) { // Incorrect number of arguments presented
        if(argc < 3)
            printf(KRED "Not enough arguments supplied\n");
        if(argc > 3)
            printf(KRED "Too many arguments supplied\n");
        return EXIT_FAILURE;
    }

    int megabytes = atoi(argv[1]);
    int trialRuns = atoi(argv[2]);

    if(megabytes < 1) {
        printf(KRED "Must input positive number of megabytes for test\n");
        return EXIT_FAILURE;
    }
    if(trialRuns < 1) {
        printf(KRED "Must input positive number of trial runs for test\n");
        return EXIT_FAILURE;
    }

    unsigned long bytes = megabytes * ONE_MEGABYTE;
    
    void *data = malloc(bytes);
    void *orig = data;

    struct timeval start, end;
    int i, j;
    const double percentChange = 1.0 / 64 / trialRuns;
    double percentage = 0;


    FILE *fp;
    fp = fopen("m64.csv", "w+");
    fprintf(fp, "Alignment, AvgTime\n");
    printf("Running on core: %d\n", sched_getcpu());
    for(i=0;i<64;i++) {
        long elapsed_time, sum = 0;
        for(j=0;j<trialRuns;j++) {
            percentage += percentChange;
            // printf("munge: 64 alignment: %d run: %d complete: %f%%\n",i,j, percentage * 100);
            gettimeofday(&start, NULL);
            Munge64(data, bytes);
            gettimeofday(&end, NULL);
            elapsed_time = ((end.tv_sec * 1000000 + end.tv_usec) -
                            (start.tv_sec * 1000000 + start.tv_usec));
            // printf("time: %ld microseconds\n", elapsed_time);
            sum += elapsed_time;
        }
        fprintf(fp, "%d, %ld\n", i, (sum/trialRuns));
        data = data + 1;
    }
    fclose(fp);
    free(orig);
    
    return 0;
}