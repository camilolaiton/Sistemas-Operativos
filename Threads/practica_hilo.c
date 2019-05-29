#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define n_hilos 5

void* funcion_maneja_hilo(void *);	//Funcion manejadora de hilo
int valida_proc_hilo(int);	//Funcion valida proc de los hijos
void error(char*);	//Funcion de error
int turno = 0;

struct parametros	//Parametros enviado a los hilos
{
	char *nombre;
	int hilo;
	char *mensaje;
};

int main(int argc, char const *argv[])
{	
	//Algoritmo panaderia, barbero, filosofos comensales
	//http://www.it.uc3m.es/pbasanta/asng/course_notes/c_threads_var_condition_es.html
	
	pthread_t pidhilo[n_hilos] = {(pthread_t)(0)};
	struct parametros *par = NULL;	//Estructura de datos para pasar a los hilos
	int i = 0;

	for (i = 0; i < n_hilos; ++i)
	{

		par = (struct parametros*)malloc(sizeof(struct parametros));
		par->nombre = "Camilo";
		par->hilo = i;
		par->mensaje = "Hola soy Camilo Laiton";

		if(valida_proc_hilo(pthread_create(&pidhilo[i], NULL, &funcion_maneja_hilo, par)))//Creation of the thread
		{
			//create worked
		}
		else
		{
			//Create didn't work
			error("No se pudo crear hilo");
		}
	}

	for (i = 0; i < n_hilos; ++i)
	{
		if(valida_proc_hilo(pthread_join(pidhilo[i], NULL)))
		{
			//El join sirvio

		}
		else
		{
			printf("Error al esperar el hilo\n");
		}
	}

	printf("Hilo principal (idthread -> [%ld]) - Hilo: [%d]\n", (long int) pthread_self(), turno);

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

void* funcion_maneja_hilo(void *param)
{
	struct parametros *persona = (struct parametros*)param;

	while(turno!=persona->hilo);

	printf("Hilo (idthread -> [%ld]) - Nombre: [%s] | hilo: [%d] | Mensaje: [%s]\n", (long int)pthread_self(), persona->nombre, persona->hilo, persona->mensaje);
	
	if(persona->hilo == n_hilos-1)
		turno = -1;
	else
		turno++;

	free(param);
	pthread_exit(0);
}