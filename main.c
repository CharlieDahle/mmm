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

int main(int argc, char *argv[])
{
	// Deal with command line arguments, save the "mode", "size" and "num threads" into globals so threads can see them
	if (argc < 3)
	{
		printf("Usage: %s <mode> <size> or %s <mode> <threads> <size>\n", argv[0], argv[0]);
		return 1;
	}

	// Determine mode
	if (strcmp(argv[1], "S") == 0)
	{
		mode = 0; // Sequential
		size = atoi(argv[2]);
		num_threads = 1; // No need for more threads in sequential mode
	}
	else if (strcmp(argv[1], "P") == 0)
	{
		if (argc < 4)
		{
			printf("Parallel mode requires <threads> and <size> arguments.\n");
			return 1;
		}
		mode = 1; // Parallel
		num_threads = atoi(argv[2]);
		size = atoi(argv[3]);
	}
	else
	{
		printf("Invalid mode. Use 'S' for Sequential or 'P' for Parallel.\n");
		return 1;
	}

	if (size <= 0 || num_threads <= 0)
	{
		printf("Size and number of threads must be positive integers.\n");
		return 1;
	}

	if (mode == 1 && num_threads > size)
	{
		printf("Number of threads should not exceed the matrix size.\n");
		return 1;
	}

	// arguments dealt with - now we start running

	printf("========\n");
	printf("mode: %s\n", mode == 0 ? "sequential" : "parallel");
	printf("thread count: %u\n", num_threads);
	printf("size: %u\n", size);
	printf("========\n");

	const int runs = 4;
	double seq_time_total = 0.0;

	// Warm-up run to initialize and populate matrices, and to warm up cache
	mmm_init();
	if (mode == 0)
	{
		mmm_seq(); // For sequential mode
	}
	else if (mode == 1)
	{ // Parallel mode
		ThreadData *threadData = malloc(num_threads * sizeof(ThreadData));
		pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

		// Create threads for parallel MMM
		for (int i = 0; i < num_threads; i++)
		{
			// Calculate and assign rows for each thread to work on
			threadData[i].start_row = i * (size / num_threads);
			threadData[i].end_row = (i + 1) * (size / num_threads);

			// Special case for the last thread to cover all remaining rows
			if (i == num_threads - 1)
			{
				threadData[i].end_row = size;
			}

			// Create each thread, passing it its portion of the matrix to work on
			pthread_create(&threads[i], NULL, mmm_par, (void *)&threadData[i]);
		}

		// Wait for all threads to complete
		for (int i = 0; i < num_threads; i++)
		{
			pthread_join(threads[i], NULL);
		}

		// Free dynamically allocated thread data and thread handles
		free(threadData);
		free(threads);
	}
	mmm_freeup(); // Clean up matrices after the warm-up run

	// timed runs

	// Sequential runs for both modes
	for (int run = 0; run < runs; run++)
	{
		mmm_init();
		double start_seq = rtclock();
		mmm_seq();
		double end_seq = rtclock();
		seq_time_total += (end_seq - start_seq);
		mmm_freeup();
	}
	double avg_seq_time = seq_time_total / runs;
	printf("Sequential Time (avg of %d runs): %.6f sec\n", runs, avg_seq_time);

	// Parallel mode execution block
	if (mode == 1)
	{
		double par_time_total = 0.0;
		mmm_init(); // Initialize once for all parallel runs

		for (int run = 0; run < runs; run++)
		{
			double start_par = rtclock();

			// Create and initialize thread data structures
			ThreadData *threadData = malloc(num_threads * sizeof(ThreadData));
			pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

			// Divide work among threads and create them
			for (int i = 0; i < num_threads; i++)
			{
				threadData[i].start_row = i * (size / num_threads);
				threadData[i].end_row = (i + 1) * (size / num_threads);

				// Ensure the last thread covers any remaining rows
				if (i == num_threads - 1)
				{
					threadData[i].end_row = size;
				}

				pthread_create(&threads[i], NULL, mmm_par, (void *)&threadData[i]);
			}

			// Join all threads
			for (int i = 0; i < num_threads; i++)
			{
				pthread_join(threads[i], NULL);
			}

			double end_par = rtclock();
			par_time_total += (end_par - start_par);
		}

		double avg_par_time = par_time_total / runs;
		printf("Parallel Time (avg of %d runs): %.6f sec\n", runs, avg_par_time);
		printf("Speedup: %.6f\n", avg_seq_time / avg_par_time);

		// Verification [I dont think this is working :( ]
		double max_diff = mmm_verify();
		printf("Verifying... largest error between parallel and sequential matrix: %.6f\n", max_diff);

		mmm_freeup(); // Free matrices after verification
	}

	return 0;
}
