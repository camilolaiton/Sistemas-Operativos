#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux
*/

/*
	Exercise
	
	The father is going to have two children where one child process
	is going to send information the other one, the information 
	that's going to be sent is saved in a file. The file have to be
	created by the user and when the other process gets the information
	it has to show it.

	NOTE: This exercise is solved in a easy way.
*/

FILE *archivo = NULL;	//Puntero file para el archivo
void generar_archivo(char *, int);	//Funcion para generar archivo
void enviar_tuberia(char *, int fd[]);	//Funcion para enviar a traves de tuberia
void leer_tuberia(int [], int);	//Funcion para leer de tuberia
void error(char *);	//Funcion de error

int main(int argc, char const *argv[])
{
	srand(time(NULL));	//Semilla

	char *filename1 = "file1.txt";	//Nombre del archivo a manejar
	int n_procesos = 2;	//Numero de procesos
	int fd[2], f = 0;	//File decriptors

	generar_archivo(filename1, 200);	//Make file

	if(pipe(fd) < 0)	//Make pipe
		exit(EXIT_FAILURE);

	switch(fork())
	{
		case -1:	//Fork is going to return -1 if there's an error, 0 if it's the child process
					//And the PID to the father
			error("No se pudo crear el proceso");
			break;

		case 0:
			close(fd[0]);	//Close read fd -> first process

			enviar_tuberia(filename1, fd);	//Send information to pipe

			close(fd[1]);	//Close write fd -> first process

			exit(EXIT_SUCCESS);	//First process -> exit
			break;

		default:	//If I am the father...

			switch(fork())
			{
				case -1:
					error("No se pudo crear el proceso");
					break;

				case 0:
					close(fd[1]);	//Second process is just going to read
									//Close write fd 
					leer_tuberia(fd, 1);	//Read from pipe

					close(fd[0]);	//Close read fd
					exit(EXIT_SUCCESS);	//Second process -> exit

					break;
			}

			close(fd[0]);	//Father closes the fd's [File Decriptor = fd]
			close(fd[1]);
			wait(NULL);	//Father waits 2 children
			wait(NULL);
			break;
	}

	return 0;
}

void leer_tuberia(int fd[], int proceso)
{
	int n = 0, bytes;
	char buffer[20] = {'\0'};

	while( (n = read(fd[0], &bytes, sizeof(bytes))) != 0)
	{
		n = read(fd[0], buffer, bytes*sizeof(char));
		printf("Proceso [%d] - Info: [%s]\n", proceso, buffer);
		memset(buffer, '\0', 20);
	}
}

void enviar_tuberia(char *filename, int fd[])
{
	if(strlen(filename) > 0)
	{
		if(archivo = fopen(filename, "r"))
		{
			char buffer[21] = {'\0'};
			int bytes = 0;

			while(!feof(archivo))
			{
				if(fscanf(archivo, "%s", buffer) == 1)
				{	
					bytes = strlen(buffer);
					write(fd[1], &bytes, sizeof(bytes));
					write(fd[1], buffer, bytes*sizeof(char));
					memset(buffer, '\0', 21);
				}
			}

			if(fclose(archivo))
			{
				perror("No se ha podido cerrar el archivo de texto");
				exit(EXIT_FAILURE);
			}

			printf("Archivo [%s] generado con exito!\n", filename);
		}
		else
		{
			perror("No se pudo abrir el archivo");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		perror("Debe tener un nombre de archivo valido");
		exit(EXIT_FAILURE);
	}
}

void generar_archivo(char *filename, int n_caracteres)
{
	if(strlen(filename) > 0)
	{
		if(archivo = fopen(filename, "w"))
		{
			fprintf(archivo, "%s\n", filename);
			char letra = ' ';

			for (int i = 0; i < n_caracteres; ++i)
			{
				letra = 32 + rand() % 94;

				if (i % 20 == 0)
					fprintf(archivo, "%c\n", letra);
				fprintf(archivo, "%c", letra);
			}

			if (fclose(archivo))
			{
				error("No se pudo cerrar el archivo");
			}
		}
		else
		{
			error("No se pudo abrir el archivo");
		}
	}
	else
	{
		error("No se pudo crear el archivo");
	}
}

void error(char *error)
{
	perror(error);
	exit(EXIT_FAILURE);
}