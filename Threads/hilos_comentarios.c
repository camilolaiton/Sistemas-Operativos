#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void *rutina_hilo(void *);	//Rutina del hilo, siempre tiene esta estructura basica

struct parametros	//Parametros a pasar al hilo por la funcionaliad -> create
{
    int n_hilo;
};

pthread_mutex_t mutex;	//Variable para el cerrojo

int contador = 0;	//variable global para ejemplo de lectura y escritura

int main(int argc, char const *argv[])	//hilo principal
{
    int n_hilos = 3;
    pthread_t *hilos = NULL;	//Puntero a los hilos para crear
    struct parametros *par = NULL;	//Puntero para los segmentos de memoria dinamica creada

    hilos = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));	//Creacion de memoria para guardar hilos
	
	if( pthread_mutex_init(&mutex, NULL) < 0 )//Se crea el cerrojo -> retorna 0 EXITO / 1 ERROR
	{
		printf("No se pudo crear el cerrojo\n");	//Mensaje por error
	}
	
    for (int i = 0; i < n_hilos; i++)	//Ciclo para crear los hilos
    {
        par = (struct parametros*)malloc(sizeof(struct parametros*));	//Creacion del segmento de memoria para que los hilos no la compartan -> ANTES DEL CREATE

        if(par)	//Si se pudo crear el segmento de memoria, retorno un valor diferente a NULL entonces puede ingresar al if
        {
            par->n_hilo = i;	//Asigno el valor al segmento de estructura de datos creada correctamente (EXISTE AL TENER DIR DE MEMORIA)
			
			if(pthread_create(&hilos[i], NULL, &rutina_hilo, par) )	//CREO EL HILO CON LOS PARAMETROS
                printf("No se pudo crear hilo %d\n", i);//MENSAJE DE ERROR O EXITO dependiendo el condicional-> Con este if sin corchetes se ejecuta la siguiente instruccion a leer por el procesador
            
        }
    }

    for (int i = 0; i < n_hilos; i++)	//Otro for para que el padre espere cada uno de los hilos creados
    {
        if(!pthread_join(hilos[i], (void*)&par))//Join -> Funcionalidad para que el hilo principal [MAIN] espere
        {
            printf("El hilo [%d] ha finalizado!\n", par->n_hilo);//Mensaje de error o exito dependiendo el condicional
            free(par);//Libero la memoria dinamica creada en for del main para crear los hilos
        }
        else
        {
            printf("No se pudo esperar hilo %d\n", i);
        }
        
    }
    
    free(hilos);//Libero la memoria dinamica para los hilos
	
	if(pthread_mutex_destroy(&mutex) < 0)	//Destruyo la variable del cerrojo
	{
		printf("Hubo un error al destruir el cerrojo\n");
	}
	
    return 0;//Finalización del hilo princiapal MAIN
}

void *rutina_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;

    printf("Hilo [%d] -> Paso \n", par->n_hilo);
    
    if(pthread_mutex_lock(&mutex))	//Inicio seccion critica con el cerrojo
    {
    	printf("No se pudo bloquear\n");
	}
    
    
	contador++; //Muchisimas instrucciones de seccion critica -> [IMPORTANTE] Seccion critica solo instrucciones de lectura y escritura
	//NADA DE FOR NI CICLOS PORQUE NO SON INSTRUCCIONES DE SECCION CRITICA
	
	
	if(pthread_mutex_unlock(&mutex))//Finalizacion de seccion con el cerrojo
    {
    	printf("No se pudo desbloquear\n");
	}
	
    pthread_exit(par);// funcionalidad para retornar un valor al hilo principal haciendo uso de la isntruccion JOIN
}
