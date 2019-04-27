#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <stdlib.h>

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux

	In this exercise we create 3 processes where each one is going to operate one part
	of the matriz, one the main diagonal, above the main diagonal and below the main diagonal.
	The father is going to show the results
*/

unsigned int sizeof_dm(int, int, size_t);
void create_index(void **, int, int, size_t);
int validar_matriz(int);
void error(char*);
void esperar_padre(int);

int main(int argc, char const *argv[])
{	/*
	typedef struct acus
	{
		int acu[3];
	};
	*/
	srand(time(NULL));
	pid_t hijo = (pid_t)(0), padre = getpid();
	int rows = 0, cols = 0, i = 0, n_procesos = 3;
	int **matriz = NULL;
	int *acu = NULL;

	int shm_idM = 0, shm_idA = 0;

	printf("Ingrese el numero de filas y columnas: (Matriz cuadrada)\n");
	scanf("%d", &rows);
	printf("\n");

	if(validar_matriz(rows))
	{
		cols = rows;

		size_t sizeMatriz = sizeof_dm(rows, cols, sizeof(int));

		shm_idM = shmget(IPC_PRIVATE, sizeMatriz, IPC_CREAT | 0600);
		shm_idA = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0600);

		if( shm_idM != -1 && shm_idA != -1)
		{
			matriz = (int**)shmat(shm_idM, NULL, 0);
			acu = (int*)shmat(shm_idA, NULL, 0);

			if(matriz && acu)
			{

				create_index((void*)matriz, rows, cols, sizeof(int));
				
				*acu = 0;

				for (i = 0; i < n_procesos; ++i)
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
					for (int k = 0; k < rows; ++k)
					{
						for (int l = 0; l < cols; ++l)
						{
							if( k > l)
							{
								matriz[k][l] = 1;// + rand() % 9;
							}
							else if( k < l)
							{
								matriz[k][l] = 2;// + rand() % 9;
							}
							else
							{
								matriz[k][l] = 0;// + rand() % 9;
							}

							printf("%d ", matriz[k][l]);
						}

						printf("\n");
					}

					esperar_padre(n_procesos);

					printf("PID: [%d] PADRE: La suma total es: [%d]\n", getpid(), *acu);

					shmdt(matriz);
					shmdt(acu);
					shmctl(shm_idA, IPC_RMID, 0);
					shmctl(shm_idM, IPC_RMID, 0);
				}
				else
				{

					sleep(2);

					int temp = 0;

					if(i == 0)
					{
						/* Sumo diagonal */

						for (int q = 0; q < rows; ++q)
						{
							temp += matriz[i][i];
						}

						printf("PID: [%d] - Valor de la diagonal: [%d]\n", getpid(), temp);
					}
					else if(i == 1)//below main diagonal
					{
						for (int c = 0; c < cols; ++c)
						{
							for (int r = 0; r < cols; ++r)
							{
								if( r > c)
								{
									temp += matriz[r][c];
								}
							}
						}

						printf("PID: [%d] - Valor izquierda diagonal: [%d]\n", getpid(), temp);
					}
					else//Above main diagonal
					{

						for (int r = 0; r < rows; ++r)
						{
							for (int c = 0; c < cols; ++c)
							{
								if( r < c)
								{
									temp += matriz[r][c];
								}
							}
						}

						printf("PID: [%d] - Valor derecha diagonal: [%d]\n", getpid(), temp);
					}

					*acu += temp;

					shmdt(matriz);
					shmdt(acu);
					exit(EXIT_SUCCESS);
				}
			}
			else
			{
				error("No se pudo agregar segmento de memoria compartida");
			}
		}
		else
		{
			error("No se pudo crear espacio de memoria compartida");
		}
	}
	else
	{
		error("Matriz no valida");
	}

	return 0;
}

void esperar_padre(int n_procesos)
{
	int ver = 0, proceso = 0;

	for (int i = 0; i < n_procesos; ++i)
	{
		proceso = wait(&ver);

		if(WIFEXITED(ver))
		{
			printf("El proceso [%d] ha terminado de manera correcta!\n", proceso);
		}
		else
		{
			printf("El proceso [%d] ha terminado de manera abrupta!\n", proceso);
		}
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

int validar_matriz(int rows)
{
	if(rows > 0)
		return 1;
	return 0;

}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}