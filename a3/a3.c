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

void create_shm(int fdr, int fdw, int *shmFd, volatile void **sharedMem)
{
	*shmFd = shm_open("/Ao32Jf8", O_CREAT | O_RDWR, 0664);
	if(*shmFd < 0) {
		write(fdw, "CREATE_SHM!", strlen("CREATE_SHM!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		unsigned int value = 0;
    		read(fdr, &value, 4);
    		ftruncate(*shmFd, value);
    		*sharedMem = (volatile void*)mmap(NULL, 1307481, PROT_READ | PROT_WRITE, MAP_SHARED, *shmFd, 0);
    		write(fdw, "CREATE_SHM!", strlen("CREATE_SHM!"));
		write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    	}
}

void write_to_shm(int fdr, int fdw, volatile void *sharedMem)
{
    	if(sharedMem == (void*)-1) {
        	write(fdw, "WRITE_TO_SHM!", strlen("WRITE_TO_SHM!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		unsigned int offset = 0;
    		unsigned int value = 0;
    		read(fdr, &offset, 4);
    		read(fdr, &value, 4);
    		if(offset >= 0 && offset <= 1307481 - 5){
    			*(unsigned int*)(sharedMem + offset) = value;
    			write(fdw, "WRITE_TO_SHM!", strlen("WRITE_TO_SHM!"));
		    	write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    		}else{
    			write(fdw, "WRITE_TO_SHM!", strlen("WRITE_TO_SHM!"));
		    	write(fdw, "ERROR!", strlen("ERROR!"));
    		}
    	}	
    	//munmap((void*)sharedMem, 1307481);
}

void map_file(int fdr, int fdw, volatile void **fileMap, int *shmFd)
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
		*fileMap = (volatile void*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, *shmFd, 0);
    		if(*fileMap == (void*)-1) {
        		write(fdw, "MAP_FILE!", strlen("MAP_FILE!"));
		    	write(fdw, "ERROR!", strlen("ERROR!"));
    		}else{
    			write(fdw, "MAP_FILE!", strlen("MAP_FILE!"));
		    	write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    		}
    	}
}

void read_from_file_offset(int fdr, int fdw, volatile void *sharedMem, volatile void *mapFile, int shmFd)
{
	if(sharedMem == (void*)-1 || mapFile == (void*)-1) {
        	write(fdw, "READ_FROM_FILE_OFFSET!", strlen("READ_FROM_FILE_OFFSET!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		unsigned int offset = 0;
    		unsigned int value = 0;
    		read(fdr, &offset, 4);
    		read(fdr, &value, 4);
    		off_t currentPosition = lseek(shmFd, 0, SEEK_CUR);
    		off_t fileSize = lseek(shmFd, 0, SEEK_END);
    		lseek(shmFd, currentPosition, SEEK_SET);
    		if(offset >= 0 && offset + value < fileSize){
    			int i = 0;
    			while(i < value){
    				char x = *(char*)(mapFile + offset + i);
    				*(char*)(sharedMem + i) = x;
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

void read_from_file_section(int fdr, int fdw, volatile void *sharedMem, volatile void *mapFile, int shmFd)
{
	if(sharedMem == (void*)-1 || mapFile == (void*)-1) {
        	write(fdw, "READ_FROM_FILE_SECTION!", strlen("READ_FROM_FILE_SECTION!"));
		write(fdw, "ERROR!", strlen("ERROR!"));
    	}else{
    		unsigned int section_no = 0;
    		unsigned int offset = 0;
    		unsigned int no_of_bytes = 0;
    		read(fdr, &section_no, 4);
    		read(fdr, &offset, 4);
    		read(fdr, &no_of_bytes, 4);
    		int x = 9;
    		unsigned int no_of_sections = 0;
    		while(*(char*)(mapFile + x) >= '0' && *(char*)(mapFile + x) <= '9'){
    			no_of_sections = no_of_sections * 10 + (*(char*)(mapFile + x) - '0');
    			x++;
    		}
    		if(section_no >= no_of_sections){
    			write(fdw, "READ_FROM_FILE_SECTION!", strlen("READ_FROM_FILE_SECTION!"));
			write(fdw, "ERROR!", strlen("ERROR!"));
    		}else{
    			unsigned int position = 20 + (section_no - 1)*19;
    			unsigned int sect_offset = *(unsigned int*)(mapFile + position);
    			unsigned int sect_size = *(unsigned int*)(mapFile + position + 4);
    			if(offset >= 0 && offset + no_of_bytes < sect_size){
    				int i = 0;
    				while(i < no_of_bytes){
    					char x = *(char*)(mapFile + sect_offset + offset + i);
    					*(char*)(sharedMem + i) = x;
    					i++;
    				}
    				write(fdw, "READ_FROM_FILE_SECTION!", strlen("READ_FROM_FILE_SECTION!"));
				write(fdw, "SUCCESS!", strlen("SUCCESS!"));
    			}else{
    				write(fdw, "READ_FROM_FILE_SECTION!", strlen("READ_FROM_FILE_SECTION!"));
				write(fdw, "ERROR!", strlen("ERROR!"));
    			}
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
	volatile void *sharedMem = NULL, *mapFile = NULL;
	
    	while(1){
    		read(fdr, &sir[i], 1);
    		if(sir[i] == '!'){
    			i++;
		    	sir[i] = '\0';
		    	if(strcmp(sir, "VARIANT!") == 0){
		    		variant(fdw);
		    	}else if(strcmp(sir, "CREATE_SHM!") == 0){
		    		create_shm(fdr, fdw, &shmFd, &sharedMem);
		    	}else if(strcmp(sir, "WRITE_TO_SHM!") == 0){
		    		write_to_shm(fdr, fdw, sharedMem);
		    	}else if(strcmp(sir, "MAP_FILE!") == 0){
		    		map_file(fdr, fdw, &mapFile, &shmFd);
		    	}else if(strcmp(sir, "READ_FROM_FILE_OFFSET!") == 0){
		    		read_from_file_offset(fdr, fdw, sharedMem, mapFile, shmFd);
		    	}else if(strcmp(sir, "READ_FROM_FILE_SECTION!") == 0){
		    		read_from_file_section(fdr, fdw, sharedMem, mapFile, shmFd);
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
