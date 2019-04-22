#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <wait.h>

#define MAX_TUB 8

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	Sistemas Operativos
	2019 - 1
*/

void generar_archivo(char *, int);
void error(char*);
int validar_num(int);
void esperar_padre(int);
int conocer_proceso(pid_t *, int);
void enviar_info(char *, int);
void leer_tub(int);
void leer_tubs(int);
int validar_sigo(int [], int);

FILE *archivo = NULL;
int **fd;

int main(int argc, char const *argv[])
{
	int cantidad = 0, proceso = 0;
	char buffer[50] = {'f', 'i', 'l', 'e', '0', '.', 't', 'x', 't'};
	pid_t father = getpid();
	pid_t *hijos = NULL;

	srand(time(NULL));

	printf("Ingrese la cantidad de procesos a crear: ");
	fflush(stdin);
	scanf("%d", &cantidad);

	if(validar_num(cantidad))
	{
		hijos = (pid_t*)malloc(cantidad*sizeof(pid_t));
		fd = (int**)malloc(cantidad*sizeof(int*));

		int ascii = 48;
		for (int i = 0; i < cantidad; ++i)
		{
			hijos[i] = (pid_t)(0);
			fd[i] = (int*)malloc(2*sizeof(int));

			fd[i][0] = 0;
			fd[i][1] = 0;

			buffer[4] = 48 + i;
			generar_archivo(buffer, 200);

		}

		for (int i = 0; i < cantidad; ++i)
		{
			if(pipe(fd[i]) < 0)
				error("No se pudo crear la tuberia");

			//printf("ciclo %d: [%d][%d]\n", i, fd[i][0], fd[i][1]);
		}
		
		for (int z = 0; z < cantidad; ++z)
		{
			hijos[z] = fork();

			if(hijos[z] == -1)
			{
				error("No se pudo crear el proceso");
			}
			else if(!hijos[z])
			{
				for (int w = 0; w < cantidad; ++w)
				{
					if(z == w)
					{
						close(fd[z][0]);
						//Dejo escritura abierta
					}
					else
					{
						close(fd[w][0]);
						close(fd[w][1]);
					}
				}

				break;
			}
		}

		if(father == getpid())
		{

			/* Soy el papa */

			for (int i = 0; i < cantidad; ++i)
			{
				close(fd[i][1]);

				//printf("PADRE: hijo %d: [%d]\n", i, hijos[i]);
			}

			esperar_padre(cantidad);

			leer_tubs(cantidad);

			for (int i = 0; i < cantidad; ++i)
			{
				//leer_tub(i);
				close(fd[i][0]);
			}
		}
		else
		{
			/* Soy algun hijo */

			proceso = conocer_proceso(hijos, cantidad);

			buffer[4] = 48 + proceso;

			//printf("Proceso [%d], buffer [%s]\n", proceso, buffer);

			enviar_info(buffer, proceso);

			close(fd[proceso][1]);
			
			free(fd);
			free(hijos);
			
			exit(EXIT_SUCCESS);
		}

	}
	else
	{
		error("Cantidad invalida");
	}

	free(fd);
	free(hijos);

	return 0;
}

void leer_tub(int n_tub)
{
	char buffer[50] = {'\0'};
	int bytes = 0, n = 0;

	while( (n = read(fd[n_tub][0], &bytes, sizeof(bytes))) != 0 )
	{
		if( (n = read(fd[n_tub][0], buffer, bytes)) < 0)
			error("Error al momento de leer");

		printf("PID: [%d] -> Recibido: [%s]\n", getpid(), buffer);

		bytes = 0;
		memset(buffer, '\0', 50);
	}
}

void enviar_info(char *filename, int n_tub)
{
	if(strlen(filename) > 0)
	{
		char buffer[50] = {'\0'};
		int bytes = 0;

		if(archivo = fopen(filename, "r"))
		{
			
			while(!feof(archivo))
			{
				if(fscanf(archivo, "%s", buffer) == 1)
				{
					bytes = strlen(buffer);

					//printf("Bytes: [%d] buffer [%s], Tub [%d]\n", bytes, buffer, n_tub);

					if(write(fd[n_tub][1], &bytes, sizeof(bytes)) < 0)
						error("No se pudo enviar info a tuberia");

					if(write(fd[n_tub][1], buffer, bytes*sizeof(char)) < 0)
						error("No se pudo enviar info a tuberia");

					printf("PID: [%d] -> Envia: [%s]\n", getpid(), buffer);

					bytes = 0;
					memset(buffer, '\0', 50);
				}
			}

			if(fclose(archivo))
				error("No se pudo cerrar el archivo de texto");

		}
		else
		{
			error("No se pudo crear el archivo de texto");
		}
	}
	else
	{
		error("Se requiere nombre para crear el archivo");
	}
}

int conocer_proceso(pid_t *hijos, int procesos)
{
	int acu = 0;

	for (int i = 0; i < procesos; ++i)
	{
		if(hijos[i] != 0)
			acu++;
	}

	return acu;
}

void esperar_padre(int procesos)
{
	int ver = 0, pro = 0;

	for (int i = 0; i < procesos; ++i)
	{
		pro = wait(&ver);

		if(WIFEXITED(ver))
		{
			printf("El proceso [%d] termino de manera correcta!\n", WIFEXITED(ver));
		}
		else
		{
			printf("El proceso [%d] termino de manera abrupta!\n", WIFEXITED(ver));
		}
	}
}

void generar_archivo(char *filename, int n_carac)
{
	if(strlen(filename) > 0)
	{
		if(archivo = fopen(filename, "w"))
		{
			char letra = ' ';

			for (int i = 1; i < n_carac; ++i)
			{
				letra = 33 + rand() % 90;

				if(i % 20 == 0)
					fprintf(archivo, "%c\n", letra);
				fprintf(archivo, "%c", letra);
			}

			fprintf(archivo, "\n%s\n", filename);

			if(fclose(archivo))
				error("No se pudo cerrar el archivo de texto");

			printf("Archivo [%s] creado exitosamente!\n", filename);

		}
		else
		{
			error("No se pudo crear el archivo de texto");
		}
	}
	else
	{
		error("Se requiere nombre para crear el archivo");
	}
}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int validar_num(int numero)
{
	if(numero > 0 && numero <= MAX_TUB)
	{
		return 1;
	}

	return 0;
}

void leer_tubs(int procesos)
{
	int *sigo = NULL, n = 0, bytes = 0;
	char buffer[50] = {'\0'};

	sigo = (int*)malloc(procesos*sizeof(int));

	for (int i = 0; i < procesos; ++i)
	{
		sigo[i] = 1;
	}

	do
	{
		for (int i = 0; i < procesos; ++i)
		{
			if(sigo[i])
			{
				if( (n = read(fd[i][0], &bytes, sizeof(bytes))) < 0)
					error("Error al momento de leer");

				if( (n = read(fd[i][0], buffer, bytes)) < 0)
					error("Error al momento de leer");

				if( n == 0)
				{
					sigo[i] = 0;
				}
				else
				{
					printf("PID: [%d] -> Recibido: [%s]\n", getpid(), buffer);

					bytes = 0;
					memset(buffer, '\0', 50);
				}
			}
		}

	}while(validar_sigo(sigo, procesos));

	free(sigo);
}

int validar_sigo(int sigo[], int cantidad)
{
	int acu = 0;

	for (int i = 0; i < cantidad; ++i)
	{
		if(sigo[i] != 0)
			acu++;	
	}

	return acu;
}