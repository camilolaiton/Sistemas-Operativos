#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <wait.h>
#include <stdlib.h>
#include <time.h>

#define LIM_ROWS 10
#define LIM_COLS 10

/*
	Camilo Laiton - 2016114073
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux
	PARCIAL
*/

void error(char *);	//Funcion de error
void esperar_padre(int);	//Funcion de esperar padre
unsigned int sizeof_dm(int, int, size_t);	//Obtener el tamaño de la matriz
void create_index(void **, int, int, size_t);	//Para usar el espacio de la memoria como matriz
int validar_num(int, int);	//Valido los numeros que ingresen
void inicializar_matriz(int **, int, int, int);	//Inicializo los valores de la matriz
void mostrar_matriz(int **, int, int);	//Muestro matriz
int multiplicar_matrices(int , int , int , int );	//Multiplico dos matrices para obtener una matriz C
int conocer_fila_ejecutar(int**, int);	//Para conocer la fila que va a ejecutar cada proc
void imprimir_trabajo_hijo(int);
int validar_filas_cols(int, int);

int **matriz_a = NULL, **matriz_b = NULL, **matriz_c = NULL;	

//Matriz a sera la matriz mxn
//Matriz b sera el vector columna
//Matriz c sera el vector resultante

int main(int argc, char const *argv[])
{
	srand(100);	//Inicializo la semilla para el random
	pid_t hijo = (pid_t)(0), padre = getpid();	//Creo las variables para validar
	int shm_id_matrices[3] = {0};	//Creo un vector para guardar los id de los espacios de memoria compartida
	int rows = 0, cols = 0, n_procesos = 0, p = 0, cantidad, n_guardar = 3;
	int sigo = 1;	//Variable para bucle infinito de proceso
	//Variables que se van a utilizar en el algoritmo

	//Rows = m <-> Cols = n

	printf("Ingrese el numero de filas de la matriz A: \n");
	scanf("%d", &rows);

	printf("Ingrese el numero de columnas de la matriz A:\n");
	scanf("%d", &cols);

	printf("Ingrese el numero de procesos a realizar la tarea: \n");
	scanf("%d", &cantidad);
	
	if(validar_num(cantidad, rows) && validar_filas_cols(rows, cols))
	{
		n_procesos = cantidad;

		size_t sizeMatriz = sizeof_dm(rows, cols, sizeof(int));
		size_t sizeColumna = sizeof_dm(cols, 1, sizeof(int));

		for (int i = 0; i < n_guardar; ++i)
		{
			if(i == 0)
			{
				shm_id_matrices[i] = shmget(IPC_PRIVATE, sizeMatriz, IPC_CREAT | 0600);
			}
			else
			{
				shm_id_matrices[i] = shmget(IPC_PRIVATE, sizeColumna, IPC_CREAT | 0600);
			}

			if(shm_id_matrices[i] == -1)//Validar si se pudo crear la memoria
			{
				error("Error al crear espacio de memoria compartida");
				return -1;
			}
		}

		matriz_a = (int**)shmat(shm_id_matrices[0], NULL, 0);//Añado el espacio a el proceso
		matriz_b = (int**)shmat(shm_id_matrices[1], NULL, 0);
		matriz_c = (int**)shmat(shm_id_matrices[2], NULL, 0);

		if(matriz_a && matriz_b && matriz_c)
		{
			create_index((void*)matriz_a, rows, cols, sizeof(int));//Consigo que se pueda usar en forma matricial
			create_index((void*)matriz_b, cols, 1, sizeof(int));
			create_index((void*)matriz_c, cols, 1, sizeof(int));

			//Las matrices columnas tendran solo 1 columna

			inicializar_matriz(matriz_a, rows, cols, 0);
			inicializar_matriz(matriz_b, cols, 1, 0);
			inicializar_matriz(matriz_c, cols, 1, 1);

			for (p = 0; p < n_procesos; ++p)
			{
				hijo = fork();

				if(hijo == -1)
				{
					error("No se pudo crear el proceso hijo");
				}
				else if(!hijo)
				{
					break;
				}
			}

			if(padre == getpid())	//Soy padre 
			{
				char instruccion[50] = {'\0'};
				sprintf(instruccion, "pstree -lp %d", getpid());	//Mostrar el arbol de procesos
				system(instruccion);

				printf("PID: [%d] PADRE -> ANTES\nPID: [%d] PADRE -> Matriz A\n", padre, padre);
				mostrar_matriz(matriz_a, rows, cols);
				printf("PID: [%d] PADRE -> vector columna B\n", padre);
				mostrar_matriz(matriz_b, cols, 1);
				printf("PID: [%d] PADRE -> vector columna C\n", padre);
				mostrar_matriz(matriz_c, cols, 1);

				esperar_padre(n_procesos);

				printf("PID[%d] PADRE -> Vector resultante: \n", getpid());
				mostrar_matriz(matriz_c, cols, 1);

				shmdt(matriz_a);//Quito espacios de memoria compartida del proceso
				shmdt(matriz_b);
				shmdt(matriz_c);

				for (int i = 0; i < n_guardar; ++i)//Elimino los espacios de memoria compartida
				{
					shmctl(shm_id_matrices[i], IPC_RMID, 0);
				}

			}
			else	//Soy alguno de los hijos
			{

				//sleep(2);

				while(sigo)
				{
					sigo = conocer_fila_ejecutar(matriz_c, cols);	//Para conocer fila a ejecutar por proceso

					if(sigo)
					{
						multiplicar_matrices(sigo-1, 0, cols, cols);	//Sigo es la fila a ejecutar por el proceso
						//mostrar_matriz(matriz_c, cols, 1);
						//sleep(5);
					}
				}

				shmdt(matriz_a);//Quito espacios de memoria compartida del proceso
				shmdt(matriz_b);
				shmdt(matriz_c);

				exit(EXIT_SUCCESS);
			}
		}
		else
		{
			error("No se pudo añadir espacios de memoria compartida");
		}
	}
	else
	{
		error("Numero de filas o columnas invalida!");
	}

	return 0;
}

int validar_filas_cols(int rows, int cols)
{
	if(rows > 0 && cols > 0 && rows <= LIM_ROWS && cols <= LIM_COLS)
		return 1;
	return 0;
}

int conocer_fila_ejecutar(int **matriz, int rows)
{
	//Retorno r+1 para poder validar el if de arriba, alla le resto 1 para procesar
	//Devuelve 0 si ya no hay mas por procesar

	for (int r = 0; r < rows; ++r)
	{
		if(matriz[r][0] == -1)
		{
			matriz[r][0] = 15;
			return r+1;
		}
	}

	return 0;
}

void imprimir_trabajo_hijo(int row_a)
{
	printf("PID[%d] -> Procesa Fila[%d] Matriz A \n", getpid(), row_a);
}

int multiplicar_matrices(int row_a, int col_b, int tam_col_a, int tam_row_b)
{
	//Funcion para multiplicar dos matrices y guardar en una tercera

	int temp = 0;
	int c_a = 0, r_b = 0;

	//printf("PID[%d] -> matriz [%d]\n", getpid(), matriz_c[row_a][col_b]);
	
	imprimir_trabajo_hijo(row_a);

	while(1)
	{
		
		temp += matriz_a[row_a][c_a] * matriz_b[r_b][col_b];

		if(c_a < tam_col_a)
			c_a++;

		if(r_b < tam_row_b)
			r_b++;

		if(c_a == tam_col_a && r_b == tam_row_b)
			break;
	}

	matriz_c[row_a][col_b] = temp;
}

void mostrar_matriz(int **matriz, int rows, int cols)
{
	//Muestro la matriz

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			printf("%d ", matriz[r][c]);
		}

		printf("\n");
	}

	printf("\n\n");
}

void inicializar_matriz(int **matriz, int rows, int cols, int cero)
{
	//Inicializar matriz

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			if(cero)
			{
				matriz[r][c] = -1;
			}
			else
			{
				matriz[r][c] = 1 + rand() % 9;
			}
		}
	}
}

int validar_num(int cantidad, int rows)
{
	//Validar para hacer proceso
	if(cantidad <= rows && cantidad > 0)
		return 1;
	return 0;
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
	//Funcion esperar padre, sirve para ver como termina el proceso

	if(procesos > 0)
	{
		int ver = 0, pid = 0;

		for (int i = 0; i < procesos; ++i)
		{
			pid = wait(&ver);

			if(WIFEXITED(ver))
			{
				//printf("El proceso [%d] termino de manera correcta! Valor: [%d]\n", pid, WIFEXITED(ver));
			}
			else
			{
				//printf("El proceso [%d] termino de manera abrupta! Valor: [%d]\n", pid, WIFEXITED(ver));
			}
		}
	}
	else
	{
		error("No se puede esperar una cantidad de procesos irreal");
	}
}

void error(char *msg)
{
	//Funcion de error
	perror(msg);
	exit(EXIT_FAILURE);
}