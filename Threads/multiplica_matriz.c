#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


void crear_cerrojo(pthread_mutex_t *);
void destruir_cerrojo(pthread_mutex_t *);
void crear_barrera(pthread_barrier_t *, int);
void destruir_barrera(pthread_barrier_t *);
void crear_condicional(pthread_cond_t *);
void destruir_condicional(pthread_cond_t *);

void error(char *);
int validar_num(int);
void inicializar_matriz(int **, int, int, int);
void mostrar_matriz(int **, int , int );
void *fnc_hilo(void *);
int **espacio_matriz(int, int);
int encontrar_num_hilos(int, int);
int multiplicar_matrices(int , int , int , int );

pthread_mutex_t mutex;

struct parametros
{
    int hilo;
    int filas;
    int columnas;
};

int **matriz_a = NULL, **matriz_b = NULL, **matriz_c = NULL;

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    int filas = 0, columnas = 0, n_hilos = 0, fila_actual = 0, columna_actual = 0;
    pthread_t *threads = NULL;
    struct parametros *par = NULL;

    printf("Ingrese la cantidad de filas de la matriz: \n");
    scanf("%d", &filas);

    printf("Ingrese la cantidad de columnas de la matriz: \n");
    scanf("%d", &columnas);

    if(validar_num(filas) && validar_num(columnas))
    {
        matriz_a = espacio_matriz(filas, columnas);
        matriz_b = espacio_matriz(filas, columnas);
        matriz_c = espacio_matriz(filas, columnas);

        threads = (pthread_t*)malloc(sizeof(pthread_t));

        n_hilos = encontrar_num_hilos(filas, columnas);

        if(matriz_a && matriz_b && matriz_c && threads && n_hilos > 0)
        {
            inicializar_matriz(matriz_a, filas, columnas, 1);
            inicializar_matriz(matriz_b, filas, columnas, 1);
            inicializar_matriz(matriz_c, filas, columnas, 0);

            mostrar_matriz(matriz_a, filas, columnas);
            mostrar_matriz(matriz_b, filas, columnas);
            mostrar_matriz(matriz_c, filas, columnas);

            crear_cerrojo(&mutex);

            for (int i = 0; i < n_hilos; i++)
            {   
                par = (struct parametros*)malloc(sizeof(struct parametros));

                if(par)
                {
                    par->hilo = i;
                    par->filas = filas;
                    par->columnas = columnas;

                    if(pthread_create(&threads[i], NULL, &fnc_hilo, par))
                        error("No se pudo crear hilo");
                }
                else
                {
                    error("No se pudo conseguir espacio para los parametros");
                }
            }

            for (int i = 0; i < n_hilos; i++)
            {   
                par = NULL;

                if(!pthread_join(threads[i], (void*)&par))
                {
                    printf("Hilo [%d] finalizado!\n", par->hilo);
                    free(par);
                }
                else
                {
                    error("Error en esperar el hilo");   
                }
                
            }

            printf("Hilo principal -> Muestra matriz resultante\n");
            mostrar_matriz(matriz_c, filas, columnas);

            destruir_cerrojo(&mutex);

            free(matriz_a);
            free(matriz_b);
            free(matriz_c);
            free(threads);
        }
        else
        {
            error("No se pudo crear espacio de memoria o # hilos no valido");
        }
        
        
    }
    else
    {
        error("Valores invalidos");
    }
    
    return 0;
}

int multiplicar_matrices(int row_a, int col_b, int tam_col_a, int tam_row_b)
{
	int temp = 0;
	int c_a = 0, r_b = 0;

	while(1)
	{
		
		temp += (matriz_a[row_a][c_a] * matriz_b[r_b][col_b]);

        if(c_a == tam_col_a-1 && r_b == tam_row_b-1) 
            break;

		if(c_a < tam_col_a-1)
			c_a++;

		if(r_b < tam_row_b-1)
			r_b++;

	}

	matriz_c[row_a][col_b] = temp;
}

int encontrar_num_hilos(int rows, int cols)
{
	int menor = 1;

	if(rows != cols)
	{
		if(rows < cols)
		{
			menor = rows;
		}
		else
		{
			menor = cols;
		}

		if(menor % 2 == 0)
		{
			return(menor/2);
		}
		else
		{
			return (menor - 1);
		}
	}
	else
	{

		menor = rows / 2;

		if(rows%2 != 0)
		{
			menor += 1;
		}

		return menor;
	}
}

int **espacio_matriz(int filas, int columnas)
{
    int **matriz = NULL;

    matriz = (int**)malloc(filas*sizeof(int*));

    for (int i = 0; i < filas; i++)
    {
        matriz[i] = (int*)malloc(columnas*sizeof(int));
    }

    return matriz;
}

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;

    int row_a_mul = par->hilo;
    int row2_a_mul = par->filas -1 - par->hilo;

    int col_a_mul = par->hilo;
    int col2_a_mul = par->columnas - 1 - par->hilo;

    for (int c = par->hilo; c < par->columnas - par->hilo; ++c)
    {
        if(pthread_mutex_lock(&mutex))
            error("No se pudo bloquear mutex");

        multiplicar_matrices(row_a_mul, c, par->columnas, par->filas);
        multiplicar_matrices(row2_a_mul, c, par->columnas, par->filas);

        printf("Hilo [%d] calcula posicion: [%d][%d]\nHilo [%d] calcula posicion: [%d][%d]\n",
        par->hilo, c, row_a_mul, par->hilo, c, row2_a_mul);

        if(pthread_mutex_unlock(&mutex))
            error("No se pudo desbloquear mutex");
    }

    if((row_a_mul+1) != row2_a_mul)
    {	

        for (int r = par->hilo+1; r < par->filas-1-par->hilo; ++r)
        {
            if(pthread_mutex_lock(&mutex))
                error("No se pudo bloquear mutex");

            multiplicar_matrices(r, col_a_mul, par->columnas, par->filas);
            multiplicar_matrices(r, col2_a_mul, par->columnas, par->filas);

            printf("Hilo [%d] calcula posicion: [%d][%d]\nHilo [%d] calcula posicion: [%d][%d]\n",
            par->hilo, r, row_a_mul, par->hilo, r, row2_a_mul);

            if(pthread_mutex_unlock(&mutex))
                error("No se pudo desbloquear mutex");
        }

    }

    pthread_exit(param);
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

void inicializar_matriz(int **matriz, int rows, int cols, int valor)
{
	//Inicializar matriz

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			if(valor)
			{
				matriz[r][c] = valor;
			}
			else
			{
				matriz[r][c] = 0;//1 + rand() % 9;
			}
		}
	}
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

void crear_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_init(cond, NULL))
        error("No se pudo crear condicional");
}
void destruir_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_destroy(cond))
        error("No se pudo eliminar condicional");
}

void crear_barrera(pthread_barrier_t *br, int n_espera)
{
    if(pthread_barrier_init(br, NULL, n_espera))
        error("No se pudo crear barrera");
}

void destruir_barrera(pthread_barrier_t *br)
{
    if(pthread_barrier_destroy(br))
        error("No se pudo destruir barrera");
}

void destruir_cerrojo(pthread_mutex_t *mt)
{
    if(pthread_mutex_destroy(mt))
		error("No se ha podido destruir cerrojo");
}

void crear_cerrojo(pthread_mutex_t *mt)
{
    if(pthread_mutex_init(mt, NULL))
        error("No se ha podido crear cerrojo");
}
