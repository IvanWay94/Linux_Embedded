#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include<time.h>

#define SLEEPING_TIME (500000)
#define SLEEPING_TIME_TEST (200000)
#define BUF_LEN		(50)
/* Globalne promenljive. */

static sem_t sem_prod;
static sem_t sem_cons;

static pthread_mutex_t bufferAccess;

static char msg[BUF_LEN];

const char* enc_path = "/dev/encrypter_project";

//default value for number of itterations
int num_iter = 10;

//true = default = normal, false = test mode
bool app_mode = true;

void generate_random_message(char *src, size_t len)
{
	char rand_len;
	int i;
	
	rand_len = rand() % 50 + 1;
	printf("Random lenght generated: %d\n", rand_len);
	fflush(stdout);
	memset(src, 0, len);
	for(i=0; i<rand_len; ++i){
		src[i] = rand() % 62 + 65;
	}
	printf("Random message generated: %s\n", src);
	fflush(stdout);
}
/* Funkcija programske niti proizvodjaca. */
void* producer (void *param)
{
    int file_desc;
    int ret_val;
    int prod_count = 0;

    srand(time(NULL));

    while (1)
    {
	sem_wait(&sem_prod);
	/* Open /dev/encrypter_project device. */
	file_desc = open(enc_path, O_RDWR);
	if(file_desc < 0)
	{
		printf("'%s' device isn't open\n", enc_path);
		printf("Try:\t1) Check does '%s' node exist\n\t2)'chmod 666 /dev/ \
		       encrypter'\n\t3) ""insmod"" encrypter module\n", enc_path);

		return (void*)-1;
	}
	printf("\n[Producer]'%s' device is successfully opened!\n", enc_path);
	fflush(stdout);

	/* Write to /dev/encrypter_project device. */
	if(prod_count < num_iter){
		pthread_mutex_lock(&bufferAccess);
		if(app_mode){
			generate_random_message(msg, BUF_LEN);
		}
		ret_val = write(file_desc, msg, strlen(msg));
		printf("[Producer] Written message: %s\n", msg);
		pthread_mutex_unlock(&bufferAccess);
		printf("[Producer] ret_val: %d\n", ret_val);			
		close(file_desc);
		sem_post(&sem_cons);
		if(app_mode){
			usleep(SLEEPING_TIME);
		}
		else{
		        usleep(SLEEPING_TIME_TEST);
		}
		prod_count++;
		if(prod_count == num_iter){
			printf("Exiting producer\n");
			break; 
		}
	}	
	else{
		close(file_desc);
		break;
	}	
    }

    return 0;
}

/* Funkcija programske niti potrosaca.*/
void* consumer (void *param)
{
    int file_desc;
    int ret_val;
    int con_count = 0;
    while (1)
    {	
	if(con_count >= num_iter)
	{
	    printf("\nExiting consumer\n");
	    break;
	}

	sem_wait(&sem_cons);	
	file_desc = open(enc_path, O_RDWR);
	if(file_desc < 0)
	{
		printf("'%s' device isn't open\n", enc_path);
		printf("Try:\t1) Check does '%s' node exist\n\t2)'chmod 666 /dev/ \
		       encrypter'\n\t3) ""insmod"" encrypter module\n", enc_path);

		return (void*)-1;
	}
	printf("[Consumer]'%s' device is successfully opened!\n", enc_path);

	pthread_mutex_lock(&bufferAccess);	
	memset(msg, 0, BUF_LEN);
	/* Read from /dev/encrypter_project device. */
	ret_val = read(file_desc, msg, BUF_LEN);
	printf("[Consumer] Read message: %s\n", msg);
	pthread_mutex_unlock(&bufferAccess);

	printf("[Consumer] ret_val: %d\n", ret_val);
	/* Close /dev/encrypter_project device. */
	close(file_desc);
	con_count++;
	sem_post(&sem_prod);
    }

    return 0;
}

/* Glavna programska nit koja formira dve programske (proizvodjac i potrosac) niti i ceka njihovo gasenje. */
int main (int argc, char *argv[])
{
    if (argc > 2) {
	fprintf(stderr, "wrong number of arguments;\n");
	fprintf(stderr, "1st argument: app mode, i.e. test or number_of iterations in normal mode \n");
	return -1;
    }
    else if(argc > 1)
    {
	size_t len = strlen("test");	
	if(!memcmp(argv[1], "test", len) || !memcmp(argv[1], "Test", len) || !memcmp(argv[1], "TEST", len)){
		app_mode = false;
		memset(msg, 0, BUF_LEN);
		memcpy(msg, "0000000001", 10);
	}
	else{
	    sscanf(argv[1], "%i", &num_iter);
	}
    }

    /* Identifikatori niti. */
    pthread_t hProducer;
    pthread_t hConsumer;

    /* Formiranje semFinishSignal semafora. */
    sem_init(&sem_prod, 0, 1);
    sem_init(&sem_cons, 0, 0);

    /* Inicijalizacija objekta iskljucivog pristupa. */
    pthread_mutex_init(&bufferAccess, NULL);

    /* Formiranje programskih niti: proizodjac i potrosac. */
    pthread_create(&hProducer, NULL, producer, 0);
    pthread_create(&hConsumer, NULL, consumer, 0);

    /* Cekanje na zavrsetak formiranih niti. */
    pthread_join(hProducer, NULL);
    pthread_join(hConsumer, NULL);
    

    /* Oslobadjanje resursa. */
    sem_destroy(&sem_prod);
    sem_destroy(&sem_cons);
    pthread_mutex_destroy(&bufferAccess);

    printf("Exiting application\n");
    fflush(stdout);
    return 0;
}
