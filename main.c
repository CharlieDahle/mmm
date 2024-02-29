#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"

// shared  globals
unsigned int mode;
unsigned int size, num_threads;
double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;

int main(int argc, char *argv[]) {

	// TODO - deal with command line arguments, save the "mode"
	// "size" and "num threads" into globals so threads can see them

	// initialize my matrices
	mmm_init();

	double clockstart, clockend;
	clockstart = rtclock();	// start the clock

	// << stuff I want to clock here >>

	clockend = rtclock(); // stop the clock
	printf("Time taken: %.6f sec\n", (clockend - clockstart));

	// free some stuff up

	return 0;
}
