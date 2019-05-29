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

int buscar_matriz(int **, int, int, int);   //Funcion para buscar numero en matriz
void error(char *); //Funcion de error
void inicializar_matriz(int **, int, int, int); //Function para inicializar matriz
void mostrar_matriz(int **, int , int );    //Funcion para mostrar matriz
void *fnc_hilo(void *); //Funcion para hilos generales
void *fnc_hilo_resultado(void *);   //Funcion para hilo resultado
int **espacio_matriz(int, int); //Funcion para crear espacio de memoria
int multiplicar_matrices(int , int , int , int );   //FUncion para multiplicar dos matrices globales
int validar_con(int, int, int); //Funcion para validar posiciones consecuentes en una matriz tamaño mXm

int **matriz_a = NULL, **matriz_b = NULL, **matriz_c = NULL;    //Putneros NULL para crear matrices
int fila_actual = 0, columna_actual = 0, sigo = 0;  //Variables usadas dentro de las fnciones hilos

pthread_cond_t cond, cond_result;   //Variables de condicion
pthread_mutex_t mutex, mutex_result;    //Variables de cerrojos
pthread_barrier_t barrier;  //Variable de barrera

struct parametros   //EStructura para pasar datos a hilos
{
    int hilo;
    int filas;
    int columnas;
    int con;
};

int main(int argc, char const *argv[])
{
    pthread_t *threads = NULL;
    int filas = 0, columnas = 0, n_hilos = 0, con = 0;
    struct parametros *par = NULL;

    printf("Ingrese tamaño matriz [mxm]: \n");
    scanf("%d", &filas);

    printf("Ingrese el numero de hilos: \n");
    scanf("%d", &n_hilos);

    printf("Numero de posiciones continuas: \n");
    scanf("%d", &con);

    if(filas > 0 && n_hilos > 1 && validar_con(con, filas, n_hilos))    //Se valida la informacion
    {
        columnas = filas;   //Matriz mxm

        n_hilos++;  //Se aumenta numero de hijos para que el ultimo hijo muestre resultado

        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));    //Espacio de memoria de los hilos

        matriz_a = espacio_matriz(filas, columnas); //Creo los espacios de memoria
        matriz_b = espacio_matriz(filas, columnas);
        matriz_c = espacio_matriz(filas, columnas);

        if(matriz_a && matriz_b && matriz_c && threads) //Veo si se pudo crear los espacios de memoria
        {

            inicializar_matriz(matriz_a, filas, columnas, 1);   //Inicializo las matrices
            inicializar_matriz(matriz_b, filas, columnas, 1);
            inicializar_matriz(matriz_c, filas, columnas, -1);

            crear_barrera(&barrier, n_hilos);   //Inicializo las variables que necesito para la sincronizacion de hilos
            crear_cerrojo(&mutex);
            crear_cerrojo(&mutex_result);
            crear_condicional(&cond);
            crear_condicional(&cond_result);

            //printf("\nVAL[%d]\n", val);

            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));    //Para saber el indice de hilo y pasar info
                
                if(par)
                {
                    par->hilo = i;
                    par->filas = par->columnas = filas;
                    par->con = con;

                    if(i == n_hilos-1)//Ultimo hilo tendra una funcion diferente
                    {
                        if(pthread_create(&threads[i], NULL, &fnc_hilo_resultado, par))
                            error("No se pudo crear hilo");
                    }
                    else
                    {
                        if(pthread_create(&threads[i], NULL, &fnc_hilo, par))   //El resto de hijos tendran funcion similar
                            error("No se pudo crear hilo");
                    }
                }
                else
                {
                    error("No se pudo crear espacio para par");
                }
                
            }

            printf("MAIN -> Cantidad de hilos creados [%d]\n", n_hilos);

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

            destruir_condicional(&cond_result); //DEstruyo todas las variables para la sincronizacion de hilos
            destruir_condicional(&cond);
            destruir_cerrojo(&mutex);
            destruir_cerrojo(&mutex_result);
            destruir_barrera(&barrier);

            free(threads);  //Libero memoria utilizada
            free(matriz_a);
            free(matriz_b);
            free(matriz_c);
        }
        else
        {
            error("No se pudo crear espacio de memoria");
        }
        
    }
    else
    {
        error("Valor invalido");
    }
    
    
    return 0;
}

int buscar_matriz(int **matriz, int filas, int columnas, int valor)
{

    for (int f = 0; f < filas; f++)
    {
        for (int c = 0; c < columnas; c++)
        {
            if(matriz[f][c] == valor)
                return 1;
        }
    }

    return 0;

}

int validar_con(int valor, int filas, int n_hilos)
{
    if(valor > 0 && valor*n_hilos <= filas*filas)
        return 1;
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

    if(pthread_mutex_lock(&mutex))
        error("No se pudo lock mutex");

    //printf("Hilo [%d] activo [W] condicional general\n", par->hilo);

    if(pthread_cond_wait(&cond, &mutex))    //Pongo a esperar todos los hilos para que esten creados todos
        error("No se pudo condicional");

    //printf("Hilo [%d] continuo actividad despues wait [1]\n", par->hilo);    

    sigo = fila_actual*columna_actual < (par->filas-1)*(par->columnas-1);

    if(pthread_mutex_unlock(&mutex))
        error("No se pudo unlock mutex");

    while (sigo)
    {

        for (int i = 0; i < par->con; i++)
        {

            printf("Hilo [%d] -> Trabaja columna [%d] - fila [%d], sigo [%d], con [%d], i [%d]\n",
            par->hilo, columna_actual, fila_actual, sigo, par->con, i);

            multiplicar_matrices(fila_actual, columna_actual, par->columnas, par->filas);

            if(columna_actual == par->columnas-1 && fila_actual < par->filas-1)
            {
                columna_actual = 0;
                fila_actual++;
            }
            else
            {
                columna_actual++;
            }

            if(columna_actual == par->columnas-1 && fila_actual == par->filas-1) 
            {
                printf("Hilo [%d] -> Trabaja columna [%d] - fila [%d], sigo [%d], con [%d], i [%d]\n",
                par->hilo, columna_actual, fila_actual, sigo, par->con, i);

                multiplicar_matrices(fila_actual, columna_actual, par->columnas, par->filas);
                //printf("Hilo [%d] activo [S] condicional resultado\n", par->hilo);

                if(pthread_cond_signal(&cond_result))   //Cuando termine de hacer el proceso llamo al proceso resultado
                    error("No se pudo condicional broadcast");

                break;
            }

        }

        if(pthread_mutex_lock(&mutex))
            error("No se pudo lock mutex");

        sigo = fila_actual*columna_actual < (par->filas-1)*(par->columnas-1);

        if(sigo)
        {
            //printf("Hilo [%d] activo [S] condicional general [PROCESO]\n", par->hilo);

            if(pthread_cond_signal(&cond))  //Activo otro hilo para que continue
                error("No se pudo condicional");

            //printf("Hilo [%d] activo [W] condicional general [PROCESO]\n", par->hilo);
            
            if(pthread_cond_wait(&cond, &mutex))    //Y espero a que otro me llame
                error("No se pudo condicional");

        }

        if(pthread_mutex_unlock(&mutex))
            error("No se pudo unlock mutex");

    }

    pthread_barrier_wait(&barrier); //Todos los hilos tienen que esperar en la barrera para luego mostrar resultado
    pthread_exit(param);
}

void *fnc_hilo_resultado(void *param)   //Se necesita necesariamente que sea el ultimo hilo creado
{
    struct parametros *par = (struct parametros*)param;

    if(pthread_mutex_lock(&mutex_result))
        error("No se pudo lock mutex");

    //printf("Hilo [%d] activo [S] condicional general\n", par->hilo);

    if(pthread_cond_signal(&cond))  //Activo el primer proceso asegurando que ya todos se crearon
        error("No se pudo condicional");

    //printf("Hilo [%d] activo [W] condicional resultado\n", par->hilo);

    if(pthread_cond_wait(&cond_result, &mutex_result))  //Pongo a esperar hilo resultado
        error("No se pudo cond_result");

    //printf("Hilo [%d] activo [S] condicional [BROADCAST]\n", par->hilo);

    if(pthread_cond_broadcast(&cond))   //Si resultado siguio es porque termino proceso y desbloquea todos los hilos
        error("No se pudo activar cond_broad");

    if(pthread_mutex_unlock(&mutex_result))
        error("No se pudo unlock mutex");

    pthread_barrier_wait(&barrier); //Todos los hilos terminan su proceso

    printf("\nHilo [%d] -> Muestra resultado, matriz C\n", par->hilo);
    mostrar_matriz(matriz_c, par->filas, par->columnas);    //Muestro matriz resultante 

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
