#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

#define MAX_PROC 4

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux
*/

int validar_cantidad(int);
void error(char *);
void esperar_padre(int);
int enviar_info();
void leer_info(int);
int reenviar(int);

int **fd = NULL;

int main(int argc, char const *argv[])
{

	int cantidad = 0, n_tubs = 0, p = 0, sigo = 1;
	pid_t padre = getpid(), hijo = (pid_t)(0);

	printf("Ingrese la cantidad de procesos a crear: \n");
	scanf("%d\n", &cantidad);

	if(validar_cantidad(cantidad))
	{
		n_tubs = cantidad*2;

		fd = (int**)malloc(n_tubs*sizeof(int*));

		for (int i = 0; i < n_tubs; ++i)
		{
			fd[i] = (int*)malloc(2*sizeof(int));

			if(pipe(fd[i]) < 0)
				error("No se pudo crear la tuberia");
		}

		for (p = 0; p < cantidad; ++p)
		{
			hijo = fork();

			if(hijo == -1)
			{
				error("No se pudo crear proceso");
			}
			else if(!hijo)
			{

				for (int t = 0; t < n_tubs; ++t)
				{
					if( t != p && t != (n_tubs-2-p) )
					{
						close(fd[t][0]);
					}

					if( t != p+1 && t != (n_tubs-1-p))
					{
						close(fd[t][1]);
					}
				}

				break;
			}
		}

		if(padre == getpid())
		{

			for (int t = 0; t < n_tubs; ++t)
			{
				
				if(t != 0)
				{
					close(fd[t][1]);
				}

				if( t != (n_tubs-1) )
				{
					close(fd[t][0]);
				}
			}

			char instruccion[50] = {'\0'};
			sprintf(instruccion,"pstree -lp %d", getpid());

			while(sigo)
			{
				system(instruccion);

				sigo = enviar_info();
				leer_info(n_tubs-1);
			}

			close(fd[0][1]);
			close(fd[n_tubs-1][0]);

			esperar_padre(cantidad);
			free(fd);
		}
		else
		{
			while(sigo)
			{
				sigo = reenviar(p);
				sigo = reenviar(n_tubs-2-p);
			}

			free(fd);
			exit(EXIT_SUCCESS);
		}
	}
	else
	{
		error("Cantidad invalida!");
	}

	return 0;
}

void esperar_padre(int procesos)
{
	int ver = 0;

	for (int i = 0; i < procesos; ++i)
	{
		wait(&ver);

		if(WIFEXITED(ver))
		{
			printf("El proceso salio de manera correcta, valor [%d]\n", WIFEXITED(ver));
		}
		else
		{
			printf("El proceso salio de manera abrupta!\n");
		}
	}
}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int validar_cantidad(int cantidad)
{
	if(cantidad > 0 && cantidad <= MAX_PROC)
		return 1;
	return 0;
}

int enviar_info()
{

	char buffer[50] = {'\0'};
	int bytes = 0;

	do
	{
		printf("PID: [%d] PADRE -> Ingrese el mensaje a enviar: \n", getpid());
		fflush(stdin);
		scanf("%[^\n]", buffer);
		bytes = strlen(buffer);
		getchar();

	}while(bytes == 0);

	if( write(fd[0][1], &bytes, sizeof(bytes)) < 0)
		error("No se pudo enviar por la tuberia");

	if( write(fd[0][1], buffer, bytes) < 0)
		error("No se pudo enviar por la tuberia");

	printf("PID: [%d] SOY PADRE -> Mensaje enviado: [%s] - Tub [0]\n", getpid(), buffer);

	if(strcmp("salir", buffer) == 0)
		return 0;

	return 1;
}

void leer_info(int n_tub)
{
	char buffer[50] = {'\0'};
	int bytes = 0, n = 0;

	if( ( n = read(fd[n_tub][0], &bytes, sizeof(bytes)) ) < 0)
		error("No se pudo leer de tuberia");

	if(n != 0)
	{
		if( ( n = read(fd[n_tub][0], buffer, bytes)) < 0)
			error("No se pudo leer de tuberia");

		printf("PID: [%d] PADRE -> Info recibida: [%s] - Tub: [%d]\n", getpid(), buffer, n_tub);
	}

}

int reenviar(int n_tub)
{
	char buffer[50] = {'\0'};
	int bytes = 0, n = 0;

	if( ( n = read(fd[n_tub][0], &bytes, sizeof(bytes)) ) < 0)
		error("No se pudo leer de tuberia");

	if( ( n = read(fd[n_tub][0], buffer, bytes)) < 0)
		error("No se pudo leer de tuberia");

	if( n != 0)
	{
		if( write(fd[n_tub+1][1], &bytes, sizeof(bytes)) < 0)
		error("No se pudo enviar por la tuberia");

		if( write(fd[n_tub+1][1], buffer, bytes) < 0)
			error("No se pudo enviar por la tuberia");

		printf("PID: [%d] -> Info recibida: [%s] - Tub: [%d]\nPID: [%d] -> Info enviada: [%s] - Tub: [%d]\n", 
		getpid(), buffer, n_tub, getpid(), buffer, n_tub+1);

		return 1;
	}
	else
	{
		close(fd[n_tub][0]);
		close(fd[n_tub+1][1]);
	}

	return 0;
}