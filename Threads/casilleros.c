#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

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

void error(char *); //Funcion de error
void inicializar_vector(int *, int, int);
void mostrar_vector(int *, int);
void encontrar_max_min(int *, int, int *, int *);
int determinar_n_hilos(int, int, int);
void *fnc_hilo(void *);
int contar_vector_intervalo(int *, int, int, int);
void llenar_vector_intervalo(int *, int *, int, int, int);
void bubble_sort(int *, int);

struct parametros   //EStructura para pasar datos a hilos
{
    int hilo;   //Numero del hilo creado
    int val_min;    //Valor minimo vector original
    int val_max;    //Valor maximo de ese vector
    int size_vector;    //Size vector original
    int *vec_ori;
    int *vec_resul;
    int size_resultado;
};

pthread_mutex_t mutex;    //Variables de cerrojos
pthread_barrier_t barrier;  //Variable de barrera

int main(int argc, char const *argv[])
{
    srand(time(NULL));  //Planto semilla para el tiempo
    int size_vector = 0, tam_casilleros = 0, n_hilos = 0;    //Variables a usar en el programa
    int val_max = 0, val_min = 0;
    int *vector_original = NULL, *vector_ordenado = NULL;   //Variables a direcciones de memoria
    pthread_t *threads = NULL;  //Variable de hilos
    struct parametros *par = NULL;

    printf("ORDENAMIENTO POR CASILLEROS\n");
    printf("Ingrese el tamaño del vector a ordenar: \n");
    scanf("%d", &size_vector);

    printf("Ingrese el tamaño de casilleros: \n");
    scanf("%d", &tam_casilleros);

    if(size_vector > 0 && tam_casilleros > 0)
    {
        vector_original = (int*)calloc(size_vector, sizeof(int));   //Creo espacios de memoria
        vector_ordenado = (int*)calloc(size_vector, sizeof(int));

        if(vector_ordenado && vector_original)
        {

            printf("Vector original\n");
            inicializar_vector(vector_original, size_vector, 50);   //Inicializo con random
            mostrar_vector(vector_original, size_vector);   //Muestro vector
            encontrar_max_min(vector_original, size_vector, &val_max, &val_min);    //Encuentro max y min del vector

            printf("Valor max: [%d], Valor min [%d]\n", val_max, val_min);
            
            n_hilos = determinar_n_hilos(val_max, val_min, tam_casilleros);

            if( val_min + (n_hilos-1)*(tam_casilleros+1)+tam_casilleros < val_max)  //Valida si el valor mayor promedio puede ser mayor que el valor maximo
            {
                n_hilos++;  //Si es asi aumento el numero de hilos para calcular los valores que hagan falta
            }

            threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

            if(threads)
            {
                crear_cerrojo(&mutex);
                crear_barrera(&barrier, n_hilos);

                for (int i = 0; i < n_hilos; i++)
                {
                    par = (struct parametros*)malloc(sizeof(struct parametros));    //Para saber el indice de hilo y pasar info
                    
                    if(par)
                    {
                        par->hilo = i;
                        par->vec_ori = vector_original;
                        par->val_min = val_min+(tam_casilleros+1)*i;
                        par->val_max = par->val_min + tam_casilleros;
                        par->size_vector = size_vector;
                        par->vec_resul = vector_ordenado;
                        par->size_resultado = 0;

                        if(pthread_create(&threads[i], NULL, &fnc_hilo, par))
                            error("No se pudo crear hilo\n");
                    
                    }
                    else
                    {
                        error("No se pudo crear espacio para par\n");
                    }
                    
                }

                int indice = 0;

                for (int i = 0; i < n_hilos; i++)   //Para que todos finalicen antes del main
                {
                    par = NULL;

                    if(!pthread_join(threads[i], (void*)&par))
                    {
                        printf("Hilo [%d] -> Ha finalizado!\n", par->hilo);

                        for (int i = 0; i < par->size_resultado; i++)
                        {
                            vector_ordenado[indice] = par->vec_resul[i];
                            indice++;
                        }
                    
                        free(par);    
                    }
                    else
                    {
                        printf("No se pudo esperar hilo\n");
                    }
                    
                }

                printf("\nHilo principal muestra vectores\n");
                mostrar_vector(vector_original, size_vector);
                mostrar_vector(vector_ordenado, size_vector);

                destruir_cerrojo(&mutex);
                destruir_barrera(&barrier);
            }
            else
            {
                error("No se pudo conseguir espacio memoria de threads\n");
            }

            free(vector_ordenado);  //Libero memoria
            free(vector_original);
        }
        else
        {
            error("No se pudo crear espacio de memoria de vectores\n");
        }
    }
    else
    {
        error("Hubo un error en el proceso\n");
    }
    
    return 0;
}

int contar_vector_intervalo(int *vector, int min, int max, int tam_vec)
{
    int contador = 0;

    if(vector)
    {  
        //printf("Entro, VALUE [%d] MIN [%d] MAX [%d]\n", vector[0], min, max);

        for (int i = 0; i < tam_vec; i++)
        {
            if(vector[i] >= min && vector[i] <= max)
            {
                contador++;
            }
        }

        //printf("Contador [%d]\n", contador);
    }
    else
    {
        error("Vector invalido en contar\n");
    }

    return contador;
    
}

void bubble_sort(int *vector, int n){
    int i,j,aux;
    if(n==1||n==0){
        return;
    }
    for(i=1;i<n;i++){
        for(j=0;j<n-1;j++){
            if(vector[j]>vector[j+1]){
               aux=vector[j];
               vector[j]=vector[j+1];
               vector[j+1]=aux;
            }
        }
    }
}

void llenar_vector_intervalo(int *vec_ori, int *vec_hilo, int min_hilo, int max_hilo, int tam_ori)
{
    if(vec_ori && min_hilo < max_hilo && min_hilo > 0 && tam_ori > 0)
    {
        if(vec_hilo)
        {
            int h = 0;

            for (int i = 0; i < tam_ori; i++)
            {
                if(vec_ori[i] >= min_hilo && vec_ori[i] <= max_hilo)
                {
                    vec_hilo[h] = vec_ori[i];
                    h++;
                }
            }
        }
        else
        {
            error("No existe vector hilo\n");
        }
    }
    else
    {
        error("Error en los parametros A\n");
    }    
}

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;
    
    int sz_vector_interno = (par->val_max) - (par->val_min);
    int *vector_interno = NULL, cant_vector = 0;

    cant_vector = contar_vector_intervalo(par->vec_ori, par->val_min, par->val_max, par->size_vector);

    printf("Hilo [%d] -> Organiza de [%d] - [%d]\n", par->hilo, par->val_min, par->val_max);

    pthread_barrier_wait(&barrier);

    if(cant_vector)
    {   
        vector_interno = (int*)malloc(cant_vector * sizeof(int));

        if(vector_interno)
        {
            llenar_vector_intervalo(par->vec_ori, vector_interno, par->val_min, par->val_max, par->size_vector);

            if(pthread_mutex_lock(&mutex))
                error("No se pudo crear lock mutex");

            //printf("Hilo [%d] -> Muestra vector lleno\n", par->hilo);
            //mostrar_vector(vector_interno, cant_vector);

            bubble_sort(vector_interno, cant_vector);

            printf("Hilo [%d] -> Muestra vector organizado\n", par->hilo);
            mostrar_vector(vector_interno, cant_vector);

            par->vec_resul = vector_interno;
            par->size_resultado = cant_vector;

            if(pthread_mutex_unlock(&mutex))
                error("No se pudo crear unlock mutex");

        }
        else
        {
            error("No se pudo allocar memoria en vector interno\n");
        }
    }
    else
    {
        printf("Hilo [%d] -> No organiza valores\n", par->hilo);
    }
    
    pthread_exit(param);
}

int determinar_n_hilos(int max, int min, int tam_casilleros)
{
    if(max > min && min > 0 && tam_casilleros > 0)
    {
        int n_hilos = (max - min) / tam_casilleros;

        printf("N_hilos: [%d]\n", n_hilos);

        return n_hilos;
    }
    else
    {
        error("Error en las variables de parametro\n");
    }
    
    return 0;
}

void encontrar_max_min(int *vector, int size, int *max, int *min)
{
    if(size > 0 && vector && max && min)
    {
        *max = *min = vector[0];

        for (int i = 0; i < size; i++)
        {
            if(vector[i] < *min)
            {
                *min = vector[i];
            }

            if(vector[i] > *max)
            {
                *max = vector[i];
            }
        }
    }
    else
    {
        error("Error en los parametros\n");
    }
    
}

void mostrar_vector(int *vector, int size)
{
    if(size > 0 && vector)
    {
        for (int i = 0; i < size; i++)
        {
            printf("[%d]", vector[i]);
        }

        printf("\n\n");
    }
    else
    {
        error("Error en el size del vector o direccion de memoria\n");
    }
    
}

void inicializar_vector(int *vector, int size, int max)
{
    if(size > 0 && vector)
    {
        for (int i = 0; i < size; i++)
        {
            vector[i] = 1 + rand() % max;
        }
    }
    else
    {
        error("Error en el size del vector o direccion de memoria\n");
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
        error("No se pudo crear condicional\n");
}
void destruir_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_destroy(cond))
        error("No se pudo eliminar condicional\n");
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
