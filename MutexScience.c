// An attempt at two co-routines which will alternately
// increment a shared variable to 100.

#include <stdio.h>
#include <pthread.h>
pthread_mutex_t MyMutex;

int Counter;
void *Incrementer(void *Parity)
{
     printf("Starting counter %d\n",*(int *)Parity);
     while (Counter < 100)
     {
          if ((Counter % 2 == 0 && *(int *)Parity == 1) || (Counter %2 != 0 && *(int *)Parity == 0))
          {
              pthread_mutex_lock(&MyMutex);
               Counter++;
              fprintf(stderr,"%u Do dee do %u\n",*(int *)Parity,Counter);
              pthread_mutex_unlock(&MyMutex);
        }
     }
     pthread_exit(NULL);
}

void main()
{
     pthread_t OddCounter, EvenCounter;
     int Parity[] = {0,1};
     pthread_mutex_init(&MyMutex,NULL);
     Counter = 0; 
     pthread_create(&OddCounter,NULL,Incrementer, &Parity[0]);
     pthread_create(&EvenCounter,NULL,Incrementer, &Parity[1]);

     pthread_join(OddCounter,NULL);
     pthread_join(EvenCounter,NULL);
}



