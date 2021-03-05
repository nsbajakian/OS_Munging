/**
 * @file master.c
 * @author Nathaniel Bajakian (nsbajakian@wpi.edu)
 * @brief Munging program from Week 5 Video Assignment
 * @version 0.1
 * @date 2021-03-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>       // timekeeping
#include <sched.h>      // threads
#include <math.h>       // pow
#include <stdbool.h>	// booleans
#include <sys/wait.h>	// wait
#include <sys/types.h>	// wait
#include <argp.h>		// used by argument parser

// colors are nice
#define KRED  "\x1B[31m"
#define ONE_BILLION  1000000000L;
#define NUM_WORD_TESTS 4 ///< Number of mx.c files/executables to run. (May add m128 later)


const char *argp_program_version = "munge v0.2";
const char *argp_program_bug_address = "nsbajakian@wpi.edu";
static char doc[] = "TODO: program description here";
// static char args_doc[] = "[master.c]...";

static struct argp_option options[] = { 
    { "serial", 's', 0, 0, "Execute munging benchmarks serially."},
    { "parallel", 'p', 0, 0, "Execute munging benchmarks in parallel."},
	{ "megabytes", 'm', "NUM_MEGABYTES", 0, "Number of megabytes to run with."},
	{ "runs", 'r', "NUM_RUNS", 0, "Number of runs to execute."},
    { 0 } 
};

struct arguments {
	enum { SERIAL, PARALLEL } mode;
	int megabytes;
	int trialRuns;
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
	struct arguments* arguments = state->input;
	switch (key) {
	case 's': arguments->mode = SERIAL; break;
	case 'p': arguments->mode = PARALLEL; break;
	case 'm': {
		if(arg == NULL) {
			printf("No megabytes value supplied, using default 10MB.\n");
			arguments->megabytes = 11;
			break;
		}
		else {
			arguments->megabytes = atoi(arg); break;
		}
	}
	case 'r': {
		if(arg == NULL) {
			printf("No number of runs supplied, using default 10 runs\n");
			arguments->trialRuns = 11;
			break;
		}
		else {
			arguments->trialRuns = atoi(arg); 
			break;
		}
	}
	case ARGP_KEY_ARG: return 0;
	default: return ARGP_ERR_UNKNOWN;
	}
	return 0;
}	

static struct argp argp = { options, parse_opt, NULL, doc, 0, 0, 0 };

bool areAllDone(bool childState[]);

int main(int argc, char **argv) {
    // argv: megabytes for trial, runs
	struct arguments arguments;
	arguments.mode = PARALLEL;	// Set default mode to parallel execution
	arguments.megabytes = 10;
	arguments.trialRuns = 10;
	
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if(arguments.mode == PARALLEL)
		puts("Mode: Parallel");
	if(arguments.mode == SERIAL)
		puts("Mode: Serial");

	int megabytes = arguments.megabytes;
	int trialRuns = arguments.trialRuns;

    if(megabytes < 1) {
        printf(KRED "Must input positive number of megabytes for test\n");
        return EXIT_FAILURE;
    }
    if(trialRuns < 1) {
        printf(KRED "Must input positive number of trial runs for test\n");
        return EXIT_FAILURE;
    }

    // Confirmation screen
    int valid = 0;
    char s = 0;
    while(!valid) {
        printf("About to average %d runs of %d Megabytes each. ", trialRuns, megabytes);
        printf("Continue? [y/n]\n");
        scanf("%s", &s);
        if(s == 'y' || s == 'n')
            valid = 1;
    }

    if(s == 'n')
        return EXIT_SUCCESS;

    int i, cPID, status;

    // TODO: make childID/childState into a struct
    int childID[NUM_WORD_TESTS];
    bool childState[NUM_WORD_TESTS];
    int parentPID = getpid();
    struct timespec absStart, raceStop, agentStop;

    // Initialize trackign arrays
    for(i = 0; i < NUM_WORD_TESTS; i++)
        childID[i] = -1;
    for(i = 0; i < NUM_WORD_TESTS; i++)
        childState[i] = true; // true = computing


    // Start race clock
	if (clock_gettime(CLOCK_REALTIME, &absStart) == -1) {
		perror("error in clock_gettime");
		exit( EXIT_FAILURE);
	}

    for (i = 0; i < NUM_WORD_TESTS; i++) {
		cPID = fork();
        if (cPID == -1) {
            perror("error during fork");
        }
		if (cPID != 0) {
			// this is the parent
			printf("[Parent]: I forked off child %d.\n", cPID);
			childID[i] = cPID;
		} else {
			// this is the child
			cPID = getpid();
            int munge = pow(2, 3+i); // m8, m16, m32, m64, etc...

			printf("	[Child, PID: %d]: Executing ", cPID);
            printf("'./m%d %d %d' command...\n", munge, megabytes, trialRuns);

			// Need to convert i to aa string for use in command
			char command[20]; // TODO: use proper buffer size here
            char arg1[10];	// TODO: use proper buffer size here, currently arbitrary
            char arg2[10];	// TODO: use proper buffer size here, currently arbitrary
			snprintf(command, sizeof(command), "m%d", munge);
            snprintf(arg1, 10, "%d", megabytes);
            snprintf(arg2, 10, "%d", trialRuns);
			// Constructs slug to execute
			char *cmd[] = { command, arg1, arg2, NULL };
			execv(cmd[0], cmd);
            perror("execv error");
			exit(EXIT_FAILURE);
		}
	}

    if (getpid() == parentPID) {
		// THIS IS THE PARENT
		while (!areAllDone(childState)) {
			for (i = 0; i < NUM_WORD_TESTS; i++) { 	// go through every agent
				if (childState[i]) {		// but only if the agent is still racing
					int cStatus = waitpid(childID[i], &status, WNOHANG); // did agent finish?
					if (cStatus > 0) {			// if yes:
						childState[i] = false;	// update agent status
						if (clock_gettime(CLOCK_REALTIME, &agentStop) == -1) {
							perror("clock gettime error");
							exit( EXIT_FAILURE);
						}
						double agentTime = (agentStop.tv_sec - absStart.tv_sec)
										+ (agentStop.tv_nsec - absStart.tv_nsec) / ONE_BILLION;
						printf("[Parent]: Child %d is done! It took %lf seconds\n", childID[i], agentTime); //TODO: have this output X.XX instead of X.000
					} else if (cStatus < 0) {
						perror("waitpid error");
					}
				}
			}
			if (!areAllDone(childState)) {
				printf("[Parent]: Execution is still ongoing.\n");
				// printRacingSlugs(childID, childState);
				sleep(1);
			}
		}
		// exited while loop, race is over
		if (clock_gettime(CLOCK_REALTIME, &raceStop) == -1) {
			perror("clock gettime error");
			exit(EXIT_FAILURE);
		}

		double raceTime = (raceStop.tv_sec  - absStart.tv_sec)
						+ (raceStop.tv_nsec - absStart.tv_nsec) / ONE_BILLION;

		printf("[Parent]: Execution over. It took %lf seconds\n", raceTime);
		exit(EXIT_SUCCESS);
	}
    perror("This code shouldn't be reachable...");
    exit(EXIT_FAILURE);
}

bool areAllDone(bool childState[]) {
	int i;
	for (i = 0; i < NUM_WORD_TESTS; i++) {
		if (childState[i])
			return false;
	}
	return true;
}