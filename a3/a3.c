#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#define PIPE_RESP "RESP_PIPE_36001"
#define PIPE_REQ "REQ_PIPE_36001"

void initialize(int *fdr, int *fdw)
{
	if(mkfifo(PIPE_RESP, 0600) != 0) {
        	perror("Could not create pipe");
        	exit(1);
    	}
    	*fdr = open(PIPE_REQ, O_RDONLY);
    	if(*fdr == -1) {
        	perror("Could not open REQ_PIPE_36001 for reading");
        	exit(1);
    	}
    	*fdw = open(PIPE_RESP, O_WRONLY);
    	if(*fdw == -1) {
        	perror("Could not open RESP_PIPE_36001 for writing");
        	exit(1);
    	}
    	
    	write(*fdw, "CONNECT!", strlen("CONNECT!"));
}

void variant(int fdw)
{
	int value = 36001;
	write(fdw, "VARIANT!", strlen("VARIANT!"));
	write(fdw, &value, sizeof(int));
	write(fdw, "VALUE!", strlen("VALUE!"));
}

void create_shm(int fdr, int fdw, int *shmFd)
{
	*shmFd = shm_open("/Ao32Jf8", O_CREAT | O_RDWR, 0664);
	if(*shmFd < 0) {
		write(fdw, "CREATE_SHM!", strlen("CREATE_SHM!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		unsigned int value = 0;
    		read(fdr, &value, 4);
    		ftruncate(*shmFd, value);
    		write(fdw, "CREATE_SHM!", strlen("CREATE_SHM!"));
		write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    	}
}

void write_to_shm(int fdr, int fdw, volatile void **sharedMem, int shmFd)
{
	*sharedMem = (volatile void*)mmap(NULL, 1307481, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    	if(*sharedMem == (void*)-1) {
        	write(fdw, "WRITE_TO_SHM!", strlen("WRITE_TO_SHM!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		unsigned int offset = 0;
    		unsigned int value = 0;
    		read(fdr, &offset, 4);
    		read(fdr, &value, 4);
    		if(offset >= 0 && offset <= 1307481 - 5){
    			*(unsigned int*)(*sharedMem + offset) = value;
    			write(fdw, "WRITE_TO_SHM!", strlen("WRITE_TO_SHM!"));
		    	write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    		}else{
    			write(fdw, "WRITE_TO_SHM!", strlen("WRITE_TO_SHM!"));
		    	write(fdw, "ERROR!", strlen("ERROR!"));
    		}
    	}	
    	//munmap((void*)sharedMem, 1307481);
}

void map_file(int fdr, int fdw, volatile void **sharedMem, int *shmFd)
{
	int j = 0;
    	size_t size;
	char nume[250] = "";
	while(1){
		read(fdr, &nume[j], 1);
		j++;
		if(nume[j-1] == '!')
			break;
	}
	nume[j-1] = '\0';
	*shmFd = open(nume, O_RDONLY, 0444);
    	if(*shmFd == -1) {
        	write(fdw, "MAP_FILE!", strlen("MAP_FILE!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		struct stat st;
    		stat(nume, &st);
    		size = st.st_size;
    		ftruncate(*shmFd, size);
		*sharedMem = (volatile void*)mmap(NULL, size, PROT_READ, MAP_SHARED, *shmFd, 0);
    		if(*sharedMem == (void*)-1) {
        		write(fdw, "MAP_FILE!", strlen("MAP_FILE!"));
		    	write(fdw, "ERROR!", strlen("ERROR!"));
    		}else{
    			write(fdw, "MAP_FILE!", strlen("MAP_FILE!"));
		    	write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    		}
    	}
    	//munmap((void*)sharedMem, size);
}

void read_from_file_offset(int fdr, int fdw, volatile void **sharedMem, int *shmFd)
{
	if(*sharedMem == (void*)-1) {
        	write(fdw, "READ_FROM_FILE_OFFSET!", strlen("READ_FROM_FILE_OFFSET!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		unsigned int offset = 0;
    		unsigned int value = 0;
    		read(fdr, &offset, 4);
    		read(fdr, &value, 4);
    		if(offset >= 0 && offset <= 1307481 - value * 4 - 1){
    			int i = 0;
    			lseek(*shmFd, 0, SEEK_SET);
    			while(i < value){
    				unsigned int x = *(unsigned int*)(*sharedMem + offset + i*4);
    				printf("BAAAA %d\n", x);
    				//*(unsigned int*)(*sharedMem + i*4) = x;
    				write(*shmFd, &x, 4);
    				i++;
    			}
    			write(fdw, "READ_FROM_FILE_OFFSET!", strlen("READ_FROM_FILE_OFFSET!"));
			write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    		}else{
    			write(fdw, "READ_FROM_FILE_OFFSET!", strlen("READ_FROM_FILE_OFFSET!"));
			write(fdw, "ERROR!", strlen("ERROR!"));
    		}
    	}
}

void close_all(int shmFd, int fdr, int fdw)
{
	close(shmFd);
	close(fdw);
    	close(fdr);
    	unlink(PIPE_RESP);
}

int main(void)
{
    	int fdr = -1, fdw = -1;

	initialize(&fdr, &fdw);
    	
    	char sir[250] = "";
    	int i = 0;
	int shmFd = -1;
	volatile void *sharedMem = NULL;
    	while(1){
    		read(fdr, &sir[i], 1);
    		if(sir[i] == '!'){
    			i++;
		    	sir[i] = '\0';
		    	if(strcmp(sir, "VARIANT!") == 0){
		    		variant(fdw);
		    	}else if(strcmp(sir, "CREATE_SHM!") == 0){
		    		create_shm(fdr, fdw, &shmFd);
		    	}else if(strcmp(sir, "WRITE_TO_SHM!") == 0){
		    		write_to_shm(fdr, fdw, &sharedMem, shmFd);
		    	}else if(strcmp(sir, "MAP_FILE!") == 0){
		    		map_file(fdr, fdw, &sharedMem, &shmFd);
		    	}else if(strcmp(sir, "READ_FROM_FILE_OFFSET!") == 0){
		    		read_from_file_offset(fdr, fdw, &sharedMem, &shmFd);
		    	}else if(strcmp(sir, "EXIT!") == 0){
		    		close_all(shmFd, fdr, fdw);
    				break;
		    	}else{
		    		close_all(shmFd, fdr, fdw);
    				break;
		    	}
		    	strcpy(sir,"");
		    	i = 0;
    		}else{
    			i++;
    		}
    	}
    	
    	return 0;
}
