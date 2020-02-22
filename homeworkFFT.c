/* Catalin-Constantin MARES - 333 CB */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <pthread.h>

#define PI M_PI

typedef double complex cplx;

int N;
cplx* values;
cplx* out;
int P;
pthread_barrier_t barrier;

/* Functia de FFT preluata de pe Rosetta */
void _fft(cplx vals[], cplx outs[], int step)
{
	if (step < N) {
		_fft(outs, vals, step * 2);
		_fft(outs + step, vals + step, step * 2);

		for (int i = 0; i < N; i += 2 * step) {
			cplx t = cexp(-I * PI * i / N) * outs[i + step];
			vals[i / 2] = outs[i] + t;
			vals[(i + N) / 2] = outs[i] - t;
		}
	}
}

/* Functia executata de thread-ul 0 cand
         se folosesc 2 thread-uri        */
void* threadFunction21(void* var) {
	/* Executa apelul recursiv */
	_fft(out, values, 2);

	/* Asteapta ca si thread-ul 1 sa termine 
	       de executat apelul recursiv       */
	pthread_barrier_wait(&barrier);

	return NULL;
}

/* Functia executata de thread-ul 1 cand
		 se folosesc 2 thread-uri        */
void* threadFunction22(void* var) {
	/* Executa apelul recursiv */
	_fft(out + 1, values + 1, 2);

	/* Asteapta ca si thread-ul 0 sa termine
		   de executat apelul recursiv       */
	pthread_barrier_wait(&barrier);

	return NULL;
}

/* Functia executata de thread-ul 0 cand
		 se folosesc 4 thread-uri        */
void* threadFunction41(void* var) {
	/* Executa apelul recursiv */
	_fft(values, out, 4);

	/* Asteapta ca toate thread-urile sa
	   termine de executat apelul recursiv */
	pthread_barrier_wait(&barrier);
	
	/* Executa for-ul din apelul recursiv cu step = 2 */
	for (int i = 0; i < N; i += 4) {
		cplx t = cexp(-I * PI * i / N) * values[i + 2];
		out[i / 2] = values[i] + t;
		out[(i + N) / 2] = values[i] - t;
	}

	/* Bariera de sincronizare - aici thread-urile 
	   2 si 3 vor fi terminat si ele de executat
	   proriile apeluri recursive - evitam astfel
	   executia in paralel a unui for in care se
		    modifica aceeasi zona de memorie      */
	pthread_barrier_wait(&barrier);

	return NULL;
}

/* Functia executata de thread-ul 1 cand
		 se folosesc 4 thread-uri        */
void* threadFunction42(void* var) {
	/* Executa apelul recursiv */
	_fft(values + 2, out + 2, 4);

	/* Asteapta ca toate thread-urile sa
	   termine de executat apelul recursiv */
	pthread_barrier_wait(&barrier);

	/* Bariera de sincronizare - aici thread-urile
	   2 si 3 vor fi terminat si ele de executat
	   proriile apeluri recursive - evitam astfel
	   executia in paralel a unui for in care se
			modifica aceeasi zona de memorie      */
	pthread_barrier_wait(&barrier);

	return NULL;
}

/* Functia executata de thread-ul 2 cand
		 se folosesc 4 thread-uri        */
void* threadFunction43(void* var) {
	/* Executa apelul recursiv */
	_fft(values + 1, out + 1, 4);


	/* Asteapta ca toate thread-urile sa
	   termine de executat apelul recursiv */
	pthread_barrier_wait(&barrier);

	/* Bariera de sincronizare - aici thread-urile
	   0 si 1 vor fi terminat deja de executat
	   for-ul de dupa apelurile recursive - evitam 
	   astfel executia in paralel a unui for in 
	   care se modifica aceeasi zona de memorie    */
	pthread_barrier_wait(&barrier);

	/* Executa for-ul din apelul recursiv cu step = 2 */
	for (int i = 0; i < N; i += 4) {
		cplx t = cexp(-I * PI * i / N) * values[i + 3];
		out[i / 2 + 1] = values[i + 1] + t;
		out[(i + N) / 2 + 1] = values[i + 1] - t;
	}

	return NULL;
}

/* Functia executata de thread-ul 3 cand
		 se folosesc 4 thread-uri        */
void* threadFunction44(void* var) {
	/* Executa apelul recursiv */
	_fft(values + 3, out + 3, 4);

	/* Asteapta ca toate thread-urile sa
	   termine de executat apelul recursiv */
	pthread_barrier_wait(&barrier);

	/* Bariera de sincronizare - aici thread-urile
	   0 si 1 vor fi terminat deja de executat
	   for-ul de dupa apelurile recursive - evitam
	   astfel executia in paralel a unui for in
	   care se modifica aceeasi zona de memorie    */
	pthread_barrier_wait(&barrier);

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

	values = (cplx*)calloc(N, sizeof(cplx));

	/* Citire date din fisier */
	for (int i = 0; i < N; i++) {
		int ret1 = fscanf(fp, "%lf", (double*)&values[i]);

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
		fprintf(fp, "%lf %lf\n", creal(values[i]), cimag(values[i]));
	}

	/* Inchidere fisier de output */
	fclose(fp);
	fp = NULL;

	free(values);
	values = NULL;
}

int main(int argc, char* argv[]) {
	int i;

	/* Preluare argumente */
	getArgs(argc, argv);

	/* Citire date din fisierul de input */
	readData(argv[1]);

	out = (cplx*)calloc(N, sizeof(cplx));

	for (i = 0; i < N; i++) {
		out[i] = values[i];
	}

	if (P == 1) {
		/* Nu se folosesc thread-uri */
		_fft(values, out, 1);
	}
	else {
		/* Initializare thread-uri */
		pthread_t tid[P];
		int thread_id[P];
		void* threadFuns[P];

		for (i = 0; i < P; i++) {
			thread_id[i] = i;
		}

		/* Initializare bariera */
		pthread_barrier_init(&barrier, NULL, P);

		/* Verificare conditie de recursivitate initiala (step = 1) */
		if (N > 1) {
			/* Constructie vector de pointeri la functie in
			     functie de numarul de thread-uri folosite  */
			if (P == 2) {
				threadFuns[0] = threadFunction21;
				threadFuns[1] = threadFunction22;
			}
			else {
				threadFuns[0] = threadFunction41;
				threadFuns[1] = threadFunction42;
				threadFuns[2] = threadFunction43; 
				threadFuns[3] = threadFunction44;
			}
			
			/* Pentru 4 thread-uri trebuie sa verificam 
			    si conditia de recursivitate (step = 2) */
			if (P == 2 || (P == 4 && N > 2)) {
				/* Creare si executie thread-uri */
				for (i = 0; i < P; i++) {
					pthread_create(&(tid[i]), NULL, threadFuns[i], &(thread_id[i]));
				}

				/* Inchidere thread-uri */
				for (i = 0; i < P; i++) {
					pthread_join(tid[i], NULL);
				}
			}

			/* Executie for din primul apel recursiv extras
			           prin recursion unrolling             */
			for (i = 0; i < N; i += 2) {
				cplx t = cexp(-I * PI * i / N) * out[i + 1];
				values[i / 2] = out[i] + t;
				values[(i + N) / 2] = out[i] - t;
			}
		}

		/* Distrugere bariera */
		pthread_barrier_destroy(&barrier);
	}

	free(out);
	out = NULL;

	/* Scriere date in fisierul de output */
	writeData(argv[2]);

	return 0;
}