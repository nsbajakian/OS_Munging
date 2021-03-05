/**
 * @file m32.c
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

void Munge32( void *data, uint64_t size ) {
    uint32_t *data32 = (uint32_t*) data;
    uint32_t *data32End = data32 + (size >> 2); /* Divide size by 4. */
    uint8_t *data8 = (uint8_t*) data32End;
    uint8_t *data8End = data8 + (size & 0x00000003); /* Strip upper 30 bits. */
    
    while( data32 != data32End ) {
        *data32++ = -*data32;
    }
    while( data8 != data8End ) {
        *data8++ = -*data8;
    }
}

int main(int argc, char** argv) {
    // megabyes, runs
    int megabytes = atoi(argv[1]);
    int trialRuns = atoi(argv[2]);

    unsigned long bytes = megabytes * ONE_MEGABYTE;
    
    void *data = malloc(bytes);
    void *orig = data;

    struct timeval start, end;
    int i, j;
    const double percentChange = 1.0 / 64 / trialRuns;
    double percentage = 0;


    FILE *fp;
    fp = fopen("m32.csv", "w+");
    fprintf(fp, "Alignment, AvgTime\n");
    printf("	[Child, PID: %d]: ", getpid());
    printf("Running on core: %d\n", sched_getcpu());
    for(i=0;i<64;i++) {
        long elapsed_time, sum = 0;
        for(j=0;j<trialRuns;j++) {
            percentage += percentChange;
            // printf("munge: 32 alignment: %d run: %d complete: %f%%\n",i,j, percentage * 100);
            gettimeofday(&start, NULL);
            Munge32(data, bytes);
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