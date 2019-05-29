#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

void* funcion_maneja_hilo(void *);	//Funcion manejadora de hilo
int valida_proc_hilo(int);	//Funcion valida proc de los hijos
void error(char*);	//Funcion de error
int validar_num(int);
int obtener_saltos(int, int);
void mostrar_array(int);	//Mostrar array

struct parametros	//Parametros enviado a los hilos
{
	int hilo;
	int inicio;
	int final;
};

int *numeros = NULL;
int *resultados = NULL;

int main(int argc, char const *argv[])
{
	srand(time(NULL));	//Semilla
	int cantidad = 0, N = 0, saltos = 0, acu = 0;
	pthread_t *pid_hilos = NULL;
	struct parametros *par = NULL;	//Estructura de datos para pasar a los hilos

	printf("Ingrese la cantidad de hilos: \n");
	scanf("%d", &cantidad);

	printf("Ingrese el tamaño del vector: \n");
	scanf("%d", &N);

	if(validar_num(cantidad) && validar_num(N) && cantidad <= N)
	{

		pid_hilos = (pthread_t*)malloc(cantidad*sizeof(pthread_t));
		numeros = (int*)malloc(N*sizeof(int));
		resultados = (int*)malloc(cantidad*sizeof(int));

		for (int i = 0; i < N; ++i)
		{
			numeros[i] = 1;//1 + rand() % 10;
		}

		mostrar_array(N);

		for (int i = 0; i < cantidad; ++i)//Creo hilos
		{

			resultados[i] = 0;

			par = (struct parametros*)malloc(sizeof(struct parametros));
			par->hilo = i;
			par->inicio = saltos;
			saltos += obtener_saltos(cantidad, N);

			if(i == cantidad-1)
				par->final = N;
			else
				par->final = saltos;

			if(valida_proc_hilo(pthread_create(&pid_hilos[i], NULL, &funcion_maneja_hilo, par)))//Creation of the thread
			{
				//create worked
			}
			else
			{
				//Create didn't work
				error("No se pudo crear hilo");
			}

		}

		for (int i = 0; i < cantidad; ++i)
		{
			if(valida_proc_hilo(pthread_join(pid_hilos[i], NULL)))
			{
				//El join sirvio

			}
			else
			{
				printf("Error al esperar el hilo\n");
			}
		}

		printf("Suma total -> Hilo principal\n");

		for (int i = 0; i < cantidad; ++i)
		{
			printf("Hilo principal - Resultado [%d]: [%d]\n", i, resultados[i]);
			acu += resultados[i];
		}

		printf("Resultado total: [%d]\n", acu);

		free(resultados);
		free(pid_hilos);
		free(numeros);

	}
	else
	{
		error("Cantidad de hilos no vaida");
	}

	return 0;
}

int obtener_saltos(int hilos, int cantidad_vector)
{
	if(hilos > 0 && cantidad_vector > 0)
	{
		int validar = 0;

		validar = cantidad_vector / hilos;

		if(hilos % 2 == 0 && hilos != cantidad_vector && cantidad_vector % 2 != 0)
			validar += 1;

		if(hilos % 2 != 0 && hilos != cantidad_vector && cantidad_vector % 2 == 0)
			validar += 1;

		return validar;
	}
	else
	{
		error("Numero invalido, no se puede obtener saltos");
	}

	return 0;
}

void mostrar_array(int limite)
{
	printf("Array de numeros generado\n");

	for (int i = 0; i < limite; ++i)
	{
		printf("[%d]", numeros[i]);
	}

	printf("\n");
}

void* funcion_maneja_hilo(void *param)
{
	struct parametros *par = (struct parametros*)param;

	//Hago proceso de suma
	printf("Hilo (idthread -> [%ld]) - Inicio: [%d] | hilo: [%d] | Final: [%d] | Resultado: [%d]\n", (long int)pthread_self(), par->inicio, par->hilo, par->final, resultados[par->hilo]);

	for (int i = par->inicio; i < par->final; ++i)
	{
		resultados[par->hilo] += numeros[i];
	}

	free(par);
	pthread_exit(0);
}

int validar_num(int num)
{
	if(num > 0)
		return 1;
	return 0;
}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int valida_proc_hilo(int value)
{
	if(value != 0)
		return 0;

	return 1;
}
