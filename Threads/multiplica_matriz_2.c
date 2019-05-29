#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux 
*/

void crear_cerrojo(pthread_mutex_t *);  //funcion para crear cerrojo
void destruir_cerrojo(pthread_mutex_t *);   //Funcion para destrur cerrojo
void crear_barrera(pthread_barrier_t *, int);   //funcion para crear barrera
void destruir_barrera(pthread_barrier_t *); //Funcion para destruir barrera
void crear_condicional(pthread_cond_t *);   //Fucnion para crear condicional
void destruir_condicional(pthread_cond_t *);    //funcion para destruir condicional
void usar_condicional_wait(pthread_cond_t*, pthread_mutex_t *, int);

void error(char *); //Funcion de error
void inicializar_matriz(int **, int, int, int);
void mostrar_matriz(int **, int , int );
void *fnc_hilo(void *);
void *fnc_hilo_extra(void *);
int **espacio_matriz(int, int);
int multiplicar_matrices(int, int, int, int);

pthread_mutex_t mutex_1, mutex_2;
pthread_cond_t cond_1, cond_2;

int fila_actual = 0, despertar = 1;

struct parametros   //EStructura para pasar datos a hilos
{
    int hilo;   //Numero del hilo creado
    int col_a;
    int fila_a;
    int fila_b;
};

int **matriz_a = NULL, **matriz_b = NULL, **matriz_c = NULL;

int main(int argc, char const *argv[])
{
    struct parametros *par = NULL;
    pthread_t *threads = NULL;  //Variable de hilos
    int filas_a = 0, columnas_a = 0, filas_bc = 0, columnas_bc = 1, n_hilos = 0;

    printf("Ingrese el numero de filas de la matriz A: \n");
    scanf("%d", &filas_a);

    printf("Ingrese el numero de columnas de la matriz A: \n");
    scanf("%d", &columnas_a);

    printf("Ingrese el numero de hilos: \n");
    scanf("%d", &n_hilos);

    if(filas_a > 0 && columnas_a > 0 && n_hilos > 1)
    {
        n_hilos++;
        printf("n_hijos [%d]\n", n_hilos);
        filas_bc = columnas_a;
        matriz_a = espacio_matriz(filas_a, columnas_a);
        matriz_b = espacio_matriz(filas_bc, columnas_bc);
        matriz_c = espacio_matriz(filas_bc, columnas_bc);
        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

        if(matriz_a && matriz_b && matriz_c && threads)
        {   
            crear_cerrojo(&mutex_2);
            crear_cerrojo(&mutex_1);
            crear_condicional(&cond_1);
            crear_condicional(&cond_2);

            inicializar_matriz(matriz_a, filas_a, columnas_a, 1);
            inicializar_matriz(matriz_b, filas_bc, columnas_bc, 1);
            inicializar_matriz(matriz_c, filas_bc, columnas_bc, -1);

            mostrar_matriz(matriz_a, filas_a, columnas_a);
            mostrar_matriz(matriz_b, filas_bc, columnas_bc);
            mostrar_matriz(matriz_c, filas_bc, columnas_bc);

            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));    //Para saber el indice de hilo y pasar info
                
                if(par)
                {
                    par->hilo = i;
                    par->fila_a = filas_a;
                    par->col_a = columnas_a;
                    par->fila_b = filas_bc;

                    if(i == n_hilos-1)
                    {    
                        if(pthread_create(&threads[i], NULL, &fnc_hilo_extra, par))
                            error("No se pudo crear hilo");
                        
                    }
                    else
                    {    
                        if(pthread_create(&threads[i], NULL, &fnc_hilo, par))
                            error("No se pudo crear hilo");   
                    }
                }
                else
                {
                    error("No se pudo crear espacio para par");
                }
                
            }

            for (int i = 0; i < n_hilos; i++)   //Para que todos finalicen antes del main
            {
                par = NULL;

                if(!pthread_join(threads[i], (void*)&par))
                {
                    printf("Hilo [%d] -> Ha finalizado!\n", par->hilo);
                    free(par);
                }
                else
                {
                    printf("No se pudo esperar hilo\n");
                }
            }

            printf("Hilo principal muestra vector columna resultante\n");
            mostrar_matriz(matriz_c, filas_bc, columnas_bc);

            destruir_condicional(&cond_2);
            destruir_condicional(&cond_1);
            destruir_cerrojo(&mutex_1);
            destruir_cerrojo(&mutex_2);

            free(threads);
            free(matriz_a);
            free(matriz_b);
            free(matriz_c);
        }
        else
        {
            error("No se pudo alocar espacio de memoria\n");
        }
        
    }
    else
    {
        error("Parametros invalidos!\n");
    }
    

    return 0;
}

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;
    
    if(pthread_mutex_lock(&mutex_1))
        error("No se pudo hacer lock en mutex\n");
            
    usar_condicional_wait(&cond_1, &mutex_1, par->hilo);

    if(pthread_mutex_unlock(&mutex_1))
        error("No se pudo hacer unlock en mutex\n");

    while (fila_actual < par->fila_a-1 || despertar)
    {
        if(pthread_mutex_lock(&mutex_1))
            error("No se pudo hacer lock en mutex\n");
            
        if(fila_actual < par->fila_a-1)
        {
            printf("Hilo [%d] -> Trabaja sobre FILA A [%d] COLUMNA B [%d]\n", par->hilo, fila_actual, 0);
            multiplicar_matrices(fila_actual, 0, par->col_a, par->fila_b);
            fila_actual++;

            //printf("Hilo [%d] -> Activo cond 1\n", par->hilo);

            if(pthread_cond_signal(&cond_1))
                error("Error para activar algun hilo\n");

            usar_condicional_wait(&cond_1, &mutex_1, par->hilo);
        }
        else if(despertar)
        {
            printf("Hilo [%d] -> Trabaja sobre FILA A [%d] COLUMNA B [%d]\n", par->hilo, fila_actual, 0);
            
            multiplicar_matrices(fila_actual, 0, par->col_a, par->fila_b);
            //printf("Hilo [%d] -> Activo cond 2\n", par->hilo);
            despertar = 0;

            if(pthread_cond_signal(&cond_2))
                error("No se pudo activar cond2");
        }

        if(pthread_mutex_unlock(&mutex_1))
            error("No se pudo hacer unlock en mutex\n");
    }

    pthread_exit(param);
}

void *fnc_hilo_extra(void *param)
{
    struct parametros *par = (struct parametros*)param;

    if(pthread_mutex_lock(&mutex_2))
        error("No se pudo hacer lock en mutex\n");

    //printf("HILO [%d] -> Activo cond 1\n", par->hilo);

    if(pthread_cond_signal(&cond_1))
        error("Error para activar algun hilo\n");

    usar_condicional_wait(&cond_2, &mutex_2, par->hilo);

    //printf("HILO [%d] -> Activo todos\n", par->hilo);
    if(pthread_cond_broadcast(&cond_1))
        error("Error al activar todos los hilos");

    if(pthread_mutex_unlock(&mutex_2))
        error("No se pudo hacer unlock en mutex\n");    

    pthread_exit(param);
}


void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
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

    if(row_a < tam_row_b)
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

void crear_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_init(cond, NULL))
        error("No se pudo crear condicional\n");
}
void destruir_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_destroy(cond))
        error("No se pudo eliminar condicional\n");
}

void usar_condicional_wait(pthread_cond_t *cd, pthread_mutex_t *mt, int hilo)
{
    //printf("Hilo [%d] -> Espero...\n", hilo);

    if(pthread_cond_wait(cd, mt))
        error("No se pudo esperar cond\n");
        
    //printf("Hilo [%d] -> Continuo ejecucion\n", hilo);
}

void crear_barrera(pthread_barrier_t *br, int n_espera)
{
    if(pthread_barrier_init(br, NULL, n_espera))
        error("No se pudo crear barrera\n");
}

void destruir_barrera(pthread_barrier_t *br)
{
    if(pthread_barrier_destroy(br))
        error("No se pudo destruir barrera\n");
}

void destruir_cerrojo(pthread_mutex_t *mt)
{
    if(pthread_mutex_destroy(mt))
		error("No se ha podido destruir cerrojo\n");
}

void crear_cerrojo(pthread_mutex_t *mt)
{
    if(pthread_mutex_init(mt, NULL))
        error("No se ha podido crear cerrojo\n");
}
