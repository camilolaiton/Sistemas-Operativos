#include <stdio.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <wait.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

unsigned int sizeof_dm(int, int, size_t);
void create_index(void **, int, int, size_t);
void esperar_padre(int);
void generar_archivo(int, int, char *);
void error(char *);
void mostrar_matriz(int **, int, int);
void subir_matriz(int **, int, int, char*);

FILE *archivo = NULL;

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux
*/

int main(int argc, char const *argv[])
{
	int rows = 6, cols = 5;
	char *filename1 = "matriz_a.txt";
	char *filename2 = "matriz_b.txt";
	char *filename3 = "matriz_c.txt";
	int n_procesos = 3, p = 0;
	int shm_id_matriz[3] = {0}; 
	int **matriz_a = NULL, **matriz_b = NULL, **matriz_c = NULL;
	pid_t padre = getpid(), hijo = (pid_t)(0);
	srand(time(NULL));

	generar_archivo(rows, cols, filename1);
	generar_archivo(rows, cols, filename2);
	generar_archivo(rows, cols, filename3);

	size_t sizeMatriz = sizeof_dm(rows, cols, sizeof(int));

	for (int i = 0; i < n_procesos; ++i)
	{
		shm_id_matriz[i] = shmget(IPC_PRIVATE, sizeMatriz, IPC_CREAT | 0600);

		if(shm_id_matriz[i] == -1)
		{
			error("No se pudo crear espacio de memoria compartida");
		}
	}
	
	matriz_a = (int**)shmat(shm_id_matriz[0], NULL, 0);
	matriz_b = (int**)shmat(shm_id_matriz[1], NULL, 0);
	matriz_c = (int**)shmat(shm_id_matriz[2], NULL, 0);

	if(matriz_a && matriz_b && matriz_c)
	{
		create_index((void*)matriz_a, rows, cols, sizeof(int));
		create_index((void*)matriz_b, rows, cols, sizeof(int));
		create_index((void*)matriz_c, rows, cols, sizeof(int));

		for (p = 0; p < n_procesos; ++p)
		{
			hijo = fork();

			if(hijo == -1)
			{
				error("No se pudo crear proceso");
			}
			else if(!hijo)
			{
				break;
			}
		}

		if(padre == getpid())
		{
			char instruccion[50] = {'\0'};
			sprintf(instruccion, "pstree -lp %d", getpid());
			system(instruccion);

			memset(instruccion, '\0', 50);
			sprintf(instruccion, "ipcs -m -i %d", getpid());
			system(instruccion);

			esperar_padre(n_procesos);

			printf("PID:[%d] -> Matriz A\n", getpid());
			mostrar_matriz(matriz_a, rows, cols);
			printf("PID:[%d] -> Matriz B\n", getpid());
			mostrar_matriz(matriz_b, rows, cols);
			printf("PID:[%d] -> Matriz C\n", getpid());
			mostrar_matriz(matriz_c, rows, cols);

			shmdt(matriz_a);
			shmdt(matriz_b);
			shmdt(matriz_c);
			
			for (int i = 0; i < n_procesos; ++i)
			{
				shmctl(shm_id_matriz[i], IPC_RMID, 0);
			}

		}
		else
		{

			sleep(1);

			if(p == 0)
			{
				printf("PID: [%d] -> Sube matriz a\n", getpid());
				subir_matriz(matriz_a, rows, cols, filename1);
			}
			else if(p == 1)
			{
				printf("PID: [%d] -> Sube matriz b\n", getpid());
				subir_matriz(matriz_b, rows, cols, filename2);
			}
			else
			{
				printf("PID: [%d] -> Sube matriz c\n", getpid());
				subir_matriz(matriz_c, rows, cols, filename3);
			}

			shmdt(matriz_a);
			shmdt(matriz_b);
			shmdt(matriz_c);

			exit(EXIT_SUCCESS);
		}
	}
	else
	{
		error("No se pudo añadir espacio de memoria compartida");
	}

	return 0;
}

void subir_matriz(int **matriz, int rows, int cols, char *filename)
{
	if(archivo = fopen(filename, "r"))
	{
		int valor = 0, iRows = 0, iCols = 0;
		rows--; cols--;

		while(!feof(archivo))
		{

			if(fscanf(archivo, "%d", &valor) == 1)
			{
				matriz[iRows][iCols] = valor;

				if(iCols == cols)
				{
					iCols = 0;
					iRows++;
				}
				else
				{
					iCols++;
				}
			}
		}

		if(fclose(archivo))
		{
			error("No se pudo cerrar el archivo de texto");
		}
	}
	else
	{
		error("No se pudo crear el archivo de texto");
	}	
}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void generar_archivo(int rows, int cols, char *filename)
{
	int cantidad = rows*cols;
	int espacio = cols;

	if( cantidad > 0 && strlen(filename) > 0)
	{
		if(archivo = fopen(filename, "w"))
		{

			int valor = 0;

			for (int i = 1; i <= cantidad; ++i)
			{

				valor = 1 + rand() % 9;

				fprintf(archivo, "%d ", valor);

				if(i % espacio == 0)
					fprintf(archivo, "\n");
			}

			if(fclose(archivo))
			{
				error("No se pudo cerrar el archivo de texto");
			}
		}
		else
		{
			error("No se pudo crear el archivo de texto");
		}
	}
	else 
	{

	}
}

unsigned int sizeof_dm(int rows, int cols, size_t sizeElement)
{
	size_t size = rows * sizeof(void*);	//Index size
	size += (cols * rows * sizeElement);	//Data size
	return size;	//return the final size
}

void create_index(void **matrix, int rows, int cols, size_t sizeElement)
{
	size_t sizeRow = cols * sizeElement;	//cols times sizeElement --> sizeElement could be double, int, char...
	matrix[0] = matrix + rows;	//

	for (int i = 1; i < rows; ++i)
	{
		matrix[i] = (matrix[i-1] + sizeRow);
	}
}

void esperar_padre(int procesos)
{
	if(procesos > 0)
	{
		int ver = 0, pid = 0;

		pid = wait(&ver);

		if(WIFEXITED(ver))
		{
			printf("El proceso [%d] termino de manera correcta! Valor: [%d]\n", pid, WIFEXITED(ver));
		}
		else
		{
			printf("El proceso [%d] termino de manera abrupta! Valor: [%d]\n", pid, WIFEXITED(ver));
		}
	}
	else
	{
		error("No se puede esperar una cantidad de procesos irreal");
	}
}

void mostrar_matriz(int **matriz, int rows, int cols)
{

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			printf("%d ", matriz[r][c]);
		}

		printf("\n");
	}

	printf("\n");
}