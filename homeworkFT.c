/* Catalin-Constantin MARES - 333 CB */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <pthread.h>

#define PI M_PI

typedef double complex cplx;

int N;
double* values;
cplx* ft_values;
int P;

/* Functia executata de thread-uri pentru 
   calculul valorilor transformatei Fourier */
void* fourierTransform(void* var) {
	/* Extragere ID pentru thread */
	int thread_id = *(int*)var;

	/* Calcul indici de start si stop pentru thread */
	int start = ceil((float)N / P) * thread_id;
	int end = fmin(ceil((float)N / P) * (thread_id + 1), N);
	
	/* Calcul interval de valori pentru fiecare thread - 
	   fiecare thread va calcula valori din propriul interval;
	   nu avem, astfel, zone critice in care 2 thread-uri sa
					scrie in acelasi timp                      */
	for (int k = start; k < end; k++) {
		for (int n = 0; n < N; n++) {
			ft_values[k] += values[n] * cexp(-2 * PI * I * k * n / N);
		}
	}

	return NULL;
}

void getArgs(int argc, char* argv[]) {
	/* Verificare numar de parametri */
	if (argc < 3) {
		printf("Not enough paramters: ./program <inputFile> <outputFile> <numThreads>\n");
		exit(1);
	}

	/* Preluare numThreads */
	P = atoi(argv[3]);
}

void readData(char* filename) {
	/* Deschidere fisier de input */
	FILE* fp = fopen(filename, "rt");

	if (fp == NULL) {
		printf("Failed to open input file.\n");

		fclose(fp);
		fp = NULL;

		exit(1);
	}

	/* Citire N */
	int ret = fscanf(fp, "%d", &N);

	if (ret != 1) {
		printf("Failed to read the number of values.\n");

		fclose(fp);
		fp = NULL;

		exit(1);
	}

	values = (double*)calloc(N, sizeof(double));
	ft_values = (cplx*)calloc(N, sizeof(cplx));

	/* Citire date din fisier */
	for (int i = 0; i < N; i++) {
		int ret1 = fscanf(fp, "%lf", &values[i]);

		if (ret1 != 1) {
			printf("Failed to read the %dth element.\n", i);

			fclose(fp);
			fp = NULL;

			free(values);
			values = NULL;

			exit(1);
		}
	}

	/* Inchidere fisier de input */
	fclose(fp);
	fp = NULL;
}

void writeData(char* filename) {
	/* Deschidere fisier de output */
	FILE* fp = fopen(filename, "wt");

	if (fp == NULL) {
		fprintf(stdout, "Failed to open output file.\n");

		fclose(fp);
		fp = NULL;

		exit(1);
	}

	/* Scriere N */
	fprintf(fp, "%d\n", N);

	/* Scriere date */
	for (int i = 0; i < N; i++) {
		fprintf(fp, "%lf %lf\n", creal(ft_values[i]), cimag(ft_values[i]));
	}

	/* Inchidere fisier de output */
	fclose(fp);
	fp = NULL;
}

int main(int argc, char* argv[]) {
	/* Preluare argumente */
	getArgs(argc, argv);

	/* Citire date din fisierul de input */
	readData(argv[1]);

	/* Initializare thread-uri */
	pthread_t tid[P];
	int thread_id[P];
	int i;

	for (i = 0; i < P; i++) {
		thread_id[i] = i;
	}

	/* Creare si executie thread-uri */
	for (i = 0; i < P; i++) {
		pthread_create(&(tid[i]), NULL, fourierTransform, &(thread_id[i]));
	}

	/* Inchidere thread-uri */
	for (i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}

	/* Scriere date in fisierul de output */
	writeData(argv[2]);

	free(values);
	values = NULL;

	free(ft_values);
	ft_values = NULL;

	return 0;
}