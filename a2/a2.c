#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include "a2_helper.h"

typedef struct {
    int id;
} TH_STRUCT2;  

typedef struct {
    int id;
} TH_STRUCT3;   

typedef struct {
    int id;
} TH_STRUCT4;   		

sem_t sem3_in;
sem_t sem3_out;
sem_t sem2;
sem_t sem4_in;
sem_t sem4_out;

int nrThreads = 0;
int note3_in = 0;
int note3_out = 0;
int note4 = 0;

void *process2_thread_function(void* arg){
	TH_STRUCT2 *ts2 = (TH_STRUCT2 *)arg;
	
    	info(BEGIN, 2, ts2->id);
    	info(END, 2, ts2->id);
    	return NULL;
}

void *process3_thread_function(void* arg){
	TH_STRUCT3 *ts3 = (TH_STRUCT3 *)arg;
	if(ts3->id == 5 || ts3->id == 3){
		sem_wait(&sem3_in);
		if(ts3->id == 5){
			info(BEGIN, 3, ts3->id);
			note3_in = 1;
		}
		else{
		while(note3_in == 0){
		}
		info(BEGIN, 3, ts3->id);
		}
    		sem_post(&sem3_in);
    		sem_wait(&sem3_out);
		if(ts3->id == 3){
			info(END, 3, ts3->id);
			note3_out = 1;
		}
		else{
		while(note3_out == 0){
		}
		info(END, 3, ts3->id);
		}
    		sem_post(&sem3_out);
	}else{
		info(BEGIN, 3, ts3->id);
    		info(END, 3, ts3->id);
	}
    	return NULL;
}

void *process4_thread_function(void* arg){
	TH_STRUCT4 *ts4 = (TH_STRUCT4 *)arg;
    	sem_wait(&sem4_in);
    	info(BEGIN, 4, ts4->id);
	nrThreads++;
	/*if(nrThreads == 4){
		if(ts4->id == 15){
			note4 = 1;
		}
	}else{
		while(note4 == 0){
		}
	}*/
	info(END, 4, ts4->id);
	nrThreads--;
	sem_post(&sem4_in);
    	return NULL;
}

int main(int argc, char **argv){
    	init();
    	pid_t P2 = -1, P3 = -1, P4 = -1, P5 = -1, P6 = -1, P7 = -1, P8 = -1, P9 = -1;
    	int waitstatus2 = -1, waitstatus3 = -1, waitstatus4 = -1, waitstatus5 = -1, waitstatus6 = -1, waitstatus7 = -1, waitstatus8 = -1, waitstatus9 = -1;
	int NrOfProcesses = 1;
	
    	info(BEGIN, 1, 0);
    	
    	NrOfProcesses++;
    	P2 = fork();
    	if(P2 == 0){
    		info(BEGIN, 2, 0);
    		
    		pthread_t tid2[6];
    		TH_STRUCT2 ts2[6];
    		
    		if(sem_init(&sem2, 0, 1) != 0) {
        		perror("Could not init the semaphore");
        		return -1;
    		}
    		
    		for(int p2i = 1; p2i <= 5; p2i++){
    			ts2[p2i].id = p2i;
        		if(pthread_create(&tid2[p2i], NULL, process2_thread_function, &ts2[p2i]) != 0){
				perror("Error creating thread");
				return 1;
			}
    		}
    		for(int p2i = 1; p2i <= 5; p2i++){
        		pthread_join(tid2[p2i], NULL);
    		}
    		
    		sem_destroy(&sem2);
    		
    		info(END, 2, 0);
    		exit(0);
    	}
    	
    	NrOfProcesses++;
    	P3 = fork();
    	if(P3 == 0){
    		info(BEGIN, 3, 0);
 
    		pthread_t tid3[6];
    		TH_STRUCT3 ts3[6];
    		
    		if(sem_init(&sem3_in, 0, 2) != 0) {
        		perror("Could not init the semaphore");
        		return -1;
    		}
    		if(sem_init(&sem3_out, 0, 2) != 0) {
        		perror("Could not init the semaphore");
        		return -1;
    		}
    		
    		for(int p3i = 1; p3i <= 5; p3i++){
    			ts3[p3i].id = p3i;
        		if(pthread_create(&tid3[p3i], NULL, process3_thread_function, &ts3[p3i]) != 0){
				perror("Error creating thread");
				return 1;
			}
    		}
    		for(int p3i = 1; p3i <= 5; p3i++){
        		pthread_join(tid3[p3i], NULL);
    		}
    		
    		sem_destroy(&sem3_in);
    		sem_destroy(&sem3_out);
    		
    		NrOfProcesses++;
    		P5 = fork();
    		if(P5 == 0){
    			info(BEGIN, 5, 0);
    			info(END, 5, 0);
    			exit(0);
    		}
    		NrOfProcesses++;
    		P6 = fork();
    		if(P6 == 0){
    			info(BEGIN, 6, 0);
    			info(END, 6, 0);
    			exit(0);
    		}
    		NrOfProcesses++;
    		P7 = fork();
    		if(P7 == 0){
    			info(BEGIN, 7, 0);
    			NrOfProcesses++;
    			P8 = fork();
    			if(P8 == 0){
    				info(BEGIN, 8, 0);
    				info(END, 8, 0);
    				exit(0);
    			}
    			waitpid(P8, &waitstatus8, 0);
    			NrOfProcesses--;
    			info(END, 7, 0);
    			exit(0);
    		}
    		NrOfProcesses++;
    		P9 = fork();
    		if(P9 == 0){
    			info(BEGIN, 9, 0);
    			info(END, 9, 0);
    			exit(0);
    		}
    		waitpid(P5, &waitstatus5, 0);
    		NrOfProcesses--;
    		waitpid(P6, &waitstatus6, 0);
    		NrOfProcesses--;
    		waitpid(P7, &waitstatus7, 0);
    		NrOfProcesses--;
    		waitpid(P9, &waitstatus9, 0);
    		NrOfProcesses--;
    		info(END, 3, 0);
    		exit(0);
    	}
    	
    	NrOfProcesses++;
    	P4 = fork();
    	if(P4 == 0){
    		info(BEGIN, 4, 0);
    		
    		pthread_t tid4[43];
    		TH_STRUCT3 ts4[43];
    		
    		if(sem_init(&sem4_in, 0, 4) != 0) {
        		perror("Could not init the semaphore");
        		return -1;
    		}
    		if(sem_init(&sem4_out, 0, 1) != 0) {
        		perror("Could not init the semaphore");
        		return -1;
    		}
    		
    		for(int p4i = 1; p4i <= 42; p4i++){
    			ts4[p4i].id = p4i;
        		if(pthread_create(&tid4[p4i], NULL, process4_thread_function, &ts4[p4i]) != 0){
				perror("Error creating thread");
				return 1;
			}
    		}
    		for(int p4i = 1; p4i <= 42; p4i++){
        		pthread_join(tid4[p4i], NULL);
    		}
    		
    		sem_destroy(&sem4_in);
    		sem_destroy(&sem4_out);
    		
    		info(END, 4, 0);
    		exit(0);
    	}
    	
    	waitpid(P2, &waitstatus2, 0);
    	NrOfProcesses--;
    	waitpid(P3, &waitstatus3, 0);
    	NrOfProcesses--;
    	waitpid(P4, &waitstatus4, 0);
    	NrOfProcesses--;
    	info(END, 1, 0);
    	
    	return 0;
}
