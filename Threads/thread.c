#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

struct char_to_print	//struct that's going to be used by the threads
{
	char c;
	int cant;
};

void* print_xs(void* param)	//Function will be used by the threads, the param
{
	struct char_to_print* p = (struct char_to_print*) param;
	int i;

	printf(" ID: [%ld]\n", pthread_self());

	for (i = 0; i < p->cant; ++i)
		fputc(p->c, stderr);
	return NULL;
}

int valida_create(int value)
{
	if(value != 0)
	{
		//Send error function
		return 0;
	}

	return 1;
}

int valida_join(int value)
{
	if(value != 0)
	{
		//Send error function
		return 0;
	}

	return 1;
}

int main(int argc, char const *argv[])
{
	pthread_t thread1_id;	//Thread 1
	pthread_t thread2_id;	//Thread 2

	struct char_to_print thread1_args;	//Structs used by the threads
	struct char_to_print thread2_args;

	thread1_args.c = 'x';
	thread1_args.cant = 5;
	
	if(valida_create(pthread_create(&thread1_id, NULL, &print_xs, &thread1_args)))//Creation of the thread
	{
		//create worked

		if(valida_join(pthread_join(thread1_id, NULL)))
		{
			//El join sirvio
		}
	}
	else
	{
		//Create didn't work
	}

	thread2_args.c = 'y';
	thread2_args.cant = 3;
	
	if(valida_create(pthread_create(&thread2_id, NULL, &print_xs, &thread2_args)))//Creation of the thread
	{
		//create worked
		if(valida_join(pthread_join(thread2_id, NULL)))
		{
			//El join sirvio
		}
	}
	else
	{
		//Create didn't work
	}

	//Concurrencia y paralelismo -->> LEER

	return 0;
}