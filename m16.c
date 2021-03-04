/**
 * @file m16.c
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
#define ONE_MEGABYTE 10000000

void Munge16( void *data, uint64_t size ) {
    uint16_t *data16 = (uint16_t*) data;
    uint16_t *data16End = data16 + (size >> 1); /* Divide size by 2. */
    uint8_t *data8 = (uint8_t*) data16End;
    uint8_t *data8End = data8 + (size & 0x00000001); /* Strip upper 31 bits. */
    
    while( data16 != data16End ) {
        *data16++ = -*data16;
    }
    while( data8 != data8End ) {
        *data8++ = -*data8;
    }
}

int main(int argc, char** argv) {
    // megabyes, runs
    int megabytes = atoi(argv[1]);
    int trialRuns = atoi(argv[2]);

    long bytes = megabytes * ONE_MEGABYTE;
    
    void *data = malloc(bytes);
    void *orig = data;

    struct timeval start, end;
    int i, j;
    const double percentChange = 1.0 / 64 / trialRuns;
    double percentage = 0;


    FILE *fp;
    fp = fopen("m16.csv", "w+");
    fprintf(fp, "Alignment, AvgTime\n");
    printf("Running on core: %d\n", sched_getcpu());
    for(i=0;i<64;i++) {
        long elapsed_time, sum = 0;
        for(j=0;j<trialRuns;j++) {
            percentage += percentChange;
            // printf("munge: 16 alignment: %d run: %d complete: %f%%\n",i,j, percentage * 100);
            gettimeofday(&start, NULL);
            Munge16(data, bytes);
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