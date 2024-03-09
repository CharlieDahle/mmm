#ifndef MMM_H_
#define MMM_H_

// shared globals
extern unsigned int mode;
extern unsigned int size, num_threads;
extern double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;

// Struct for passing work range information to threads
typedef struct
{
    int start_row;
    int end_row;
} ThreadData;

void mmm_init();
void mmm_reset(double **);
void mmm_freeup();
void mmm_seq();
void *mmm_par(void *);
double mmm_verify();

#endif /* MMM_H_ */
