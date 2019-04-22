#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	Sistemas Operativos
	2019 - 1
*/

void generar_archivo(char *, int);
void esperar_padre(int);
void error(char*);
void leer_tub(int);
int conocer_proceso(pid_t [], int);
void enviar_info(char *, int);

FILE *archivo = NULL;
int fd[2][2] = {0};

int main(int argc, char const *argv[])
{
	srand(time(NULL));
	int n_procesos = 2, proceso = 0;
	pid_t padre = getpid();
	pid_t hijos[2] = {(pid_t)(0)};

	char *filename = "contar_caracteres.c";

	//generar_archivo(filename, 200);

	for (int i = 0; i < n_procesos; ++i)
	{
		if(pipe(fd[i]) < 0)
			error("No se pudo crear la tuberia");
	}

	for (int i = 0; i < n_procesos; ++i)
	{
		hijos[i] = fork();

		if(hijos[i] == -1)
		{
			error("No se pudo crear el proceso");
		}
		else if(!hijos[i])
		{
			for (int w = 0; w < n_procesos; ++w)
			{	
				if(i != w)
				{
					close(fd[w][0]);
					close(fd[w][1]);
				}
				else if(i == w)
				{
					close(fd[w][0]);
				}
			}

			break;
		}
	}

	if(padre == getpid())
	{	
		for (int i = 0; i < n_procesos; ++i)
		{
			//printf("Hijo [%d]\n", hijos[i]);
			close(fd[i][1]);
		}

		esperar_padre(n_procesos);

		for (int i = 0; i < n_procesos; ++i)
		{
			leer_tub(i);
			close(fd[i][0]);
		}

	}
	else
	{
		/*
		for (int i = 0; i < n_procesos; ++i)
		{
			printf("PID: [%d] -> Hijo [%d]\n", getpid(), hijos[i]);
		}
		*/

		proceso = conocer_proceso(hijos, n_procesos);
		enviar_info(filename, proceso);
		close(fd[proceso][1]);
		exit(EXIT_SUCCESS);
	}

	return 0;
}

int conocer_proceso(pid_t hijos[], int cantidad)
{
	int acu = 0;

	for (int i = 0; i < cantidad; ++i)
	{
		if(hijos[i] != 0)
			acu++;	
	}

	return acu;
}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
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

void leer_tub(int n_tub)
{
	int bytes = 0, n = 0;
	int dato = 0;

	if( (n = read(fd[n_tub][0], &dato, sizeof(int))) < 0)
		error("No se pudo leer de la tuberia");

	if(n_tub == 0)
		printf("PID: [%d] -> Cantidad de [mayusculas] en el archivo: [%d]\n", getpid(), dato);
	else
		printf("PID: [%d] -> Cantidad de [minusculas] en el archivo: [%d]\n", getpid(), dato);
}

void enviar_info(char *filename, int n_tub)
{
	char buffer[50] = {'\0'};
	int bytes = 0, cantidad = 0;
	char *buscar = NULL;

	if(n_tub == 0)
	{
		buscar = "mayusculas";
	}
	else
	{
		buscar = "minusculas";
	}

	if(strlen(filename) > 0)
	{
		if(archivo = fopen(filename, "r"))
		{

			while(!feof(archivo))
			{
				if(fscanf(archivo, "%s", buffer) == 1)
				{

					bytes = strlen(buffer);

					for (int i = 0; i < bytes; ++i)
					{
						if(n_tub == 0)
						{
							if(buffer[i] >= 'A' && buffer[i] <= 'Z')
							{
								cantidad++;
							}
						}
						else
						{
							if(buffer[i] >= 'a' && buffer[i] <= 'z')
							{
								cantidad++;
							}
						}
					}

					bytes = 0;
					memset(buffer, '\0', 50);
				}
			}

			if(write(fd[n_tub][1], &cantidad, sizeof(cantidad)) < 0)
				error("No se pudo enviar por la tuberia");


			printf("PID: [%d] -> Fui a buscar [%s] y encontre [%d]\n", getpid(), buscar, cantidad);

			if(fclose(archivo))
				error("No se pudo cerrar el archivo de texto");
		}
		else
		{
			error("No se pudo abrir el archivo");
		}
	}
	else
	{
		error("Nombre de archivo no valido");
	}
}
