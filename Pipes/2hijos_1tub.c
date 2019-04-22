#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

FILE *archivo = NULL;
void generar_archivo(char *, int);
void enviar_tuberia(char *, int fd[]);
void leer_tuberia(int [], int);
void error(char *);

int main(int argc, char const *argv[])
{
	srand(time(NULL));

	char *filename1 = "file1.txt";
	int n_procesos = 2;
	int fd[2], f;

	generar_archivo(filename1, 200);

	if(pipe(fd) < 0)
		exit(EXIT_FAILURE);

	switch(fork())
	{
		case -1:
			error("No se pudo crear el proceso");
			break;

		case 0:
			close(fd[0]);

			enviar_tuberia(filename1, fd);

			close(fd[1]);

			exit(EXIT_SUCCESS);
			break;

		default:

			switch(fork())
			{
				case -1:
					error("No se pudo crear el proceso");
					break;

				case 0:
					close(fd[1]);

					leer_tuberia(fd, 1);

					close(fd[0]);
					exit(EXIT_SUCCESS);

					break;
			}

			close(fd[0]);
			close(fd[1]);
			wait(NULL);
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