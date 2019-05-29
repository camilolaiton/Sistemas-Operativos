#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>

#define LIMITE 20

int validar_num(int);
void error(char *);
int valida_thread(int);
void *thread_fnc(void *);

struct parametros
{
	int hilo;
	int *matriz;
};

int main(int argc, char const *argv[])
{

	struct parametros *par = NULL;
	int n = 0, m = 0, n_hilos = 0;	//Variables used in the program
	int *matriz = NULL;	//Pointer to the space required
	pthread_t *threads = NULL;	//Pointer for the threads
	
	printf("NOTA: Matriz nxm (n puede ser igual a m)\n");

	printf("Ingresa el numero de filas (n): \n");
	scanf("%d", &n);

	printf("Ingresa el numero de columnas (m): \n");
	scanf("%d", &m);

	printf("Ingrese el numero de hilos: \n");
	scanf("%d", &n_hilos);

	if(validar_num(n) && validar_num(m) && n_hilos <= n)	//Validate values
	{
		matriz = (int**)malloc(n*sizeof(int*));	//Create space memory

		for (int i = 0; i < n; ++i)
		{
			matriz[i] = (int*)malloc(m*sizeof(int));
		}

		threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));	//Create the threads space

		if(matriz && threads)
		{

			char buff[20] = {'\0'};

			for (int i = 0; i < n_hilos; ++i)
			{

				par = (struct parametros*)malloc(sizeof(struct parametros));
				par->hilo = i;
				par->matriz = matriz;

				if(valida_thread(pthread_create(&threads[i], NULL, &thread_fnc, par)))
				{
					printf("Thread [%d] creado!\n", i);
				}
				else
				{
					sprintf(buff, "Hilo [%d] no pudo ser creado", i);
					error(buff);
				}
			}

			free(threads);
			free(matriz);
		}
		else
		{
			error("No se pudo obtener espacio de memoria");
		}
	}
	else
	{
		error("Tamaño de matriz invalido!");
	}

	return 0;
}

void *thread_fnc(void *param)
{
	free(param);
}

int valida_thread(int value)
{
	if(value != 0)
		return 0;
	return 1;
}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int validar_num(int num)
{
	if(num > 0 && num < LIMITE)
		return 1;
	else
		return 0;
}