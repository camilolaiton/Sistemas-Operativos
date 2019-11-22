#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void *fnc(void*);

struct param
{
    int n_hilo;
};

int contador = 0;
pthread_mutex_t mutex;

int main(int argc, char const *argv[])
{
    struct param *p  = NULL;
    pthread_t *hilos = 0;
    int n_hilos = 2000;

    if(pthread_mutex_init(&mutex, NULL))
    {
        printf("No se pudo crear el cerrojo\n");
    }
    
    hilos = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

    if(hilos)
    {

        for (int i = 0; i < n_hilos; i++)
        {
            p = (struct param*)malloc(sizeof(p));
            p->n_hilo = i;

            if(pthread_create(&hilos[i], NULL, &fnc, p))
            {
                printf("No se pudo crear hilo [%d]\n", i);
                exit(EXIT_FAILURE);
            }
        }
        
        for (int i = 0; i < n_hilos; i++)
        {
            if(pthread_join(hilos[i], (void*)&p))
            {
                printf("No se pudo esperar hilo [%d]\n", i);
                exit(EXIT_FAILURE);
            }
            else
            {
                free(p);
            }
            
        }
        
        printf("Contador finalizo: [%d]\n", contador);

        free(hilos);

        /*
        if(pthread_mutex_destroy(&mutex))
        {
            printf("Error al destruir el mutex\n");
        }*/
    }
    else
    {
        printf("No se pudo crear espacio de hilos\n");
    }
    

    return 0;
}

void *fnc(void *param)
{
    struct param *p = (struct param*)param;

    printf("Soy hilo [%d] ->\n", p->n_hilo);
    /*
    if(pthread_mutex_lock(&mutex))
        printf("Error\n");
    */
    contador++;
    /*
    if(pthread_mutex_unlock(&mutex))
        printf("Error\n");
    */
    pthread_exit(param);
}