#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void list(int op_recursive, off_t op_size_smaller, char* op_permission, const char *dirPath, int *first)
{
	DIR *dir = NULL;
    	struct dirent *entry = NULL;
    	char filePath[512];
    	struct stat statbuf;
    	dir = opendir(dirPath);
    	int perm = 00;
    	for(int i = 0; i < strlen(op_permission); i++){
    		if(op_permission[i] != '-'){
    			if(i % 3 == 0)
    				perm += 04;
    			else if(i % 3 == 1)
    				perm += 02;
    			else if(i % 3 == 2)
    				perm += 01;
    		}
    		if(i % 3 == 2 && i != strlen(op_permission) -1)
    			perm *= 8;
    	}
    	if(dir == NULL) {
        	printf("ERROR\nNu se poate deschide directorul");
    	}
    	else{
    		if(*first != 0){
    			printf("SUCCESS\n");
    			*first = 0;
    		}
    		while((entry = readdir(dir)) != NULL) {
    			snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
    			if(lstat(filePath, &statbuf) == 0) {
            			if(S_ISREG(statbuf.st_mode)) {
                			if(op_size_smaller != 0 && strcmp(op_permission,"") != 0){
                				if(statbuf.st_size < op_size_smaller && perm == statbuf.st_mode % 512)
                					printf("%s\n",filePath);
                			}else if(op_size_smaller != 0){
                		 		if(statbuf.st_size < op_size_smaller)
                		 			printf("%s\n",filePath);
                			}else if(strcmp(op_permission,"") != 0){
                				if(perm == statbuf.st_mode % 512)
                					printf("%s\n",filePath);
                			}else{
                				printf("%s\n",filePath);
                			}
            			} else if(S_ISDIR(statbuf.st_mode)){
            		 		if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            		 			if(strcmp(op_permission,"") != 0){
            		 				if(perm == statbuf.st_mode % 512)
                						printf("%s\n",filePath);
                				}else if(op_size_smaller == 0){
                					printf("%s\n",filePath);
                				}
                				if(op_recursive == 1){
                					list(op_recursive, op_size_smaller, op_permission, filePath, first);
                				}
                			}
            			}
        		}
    		}
    	}
    	closedir(dir);
}

void parse(const char *filePath)
{
	int fd = -1;
	char buff[100] = "";
	fd = open(filePath, O_RDONLY);
	if(fd == -1){
		printf("ERROR\nFisierul nu a putut fi deschis\n");
	}else{
		read(fd, buff, 100 * sizeof(char));
		printf("%s\n", buff);
		/*char buff2[10] = "";
		strcpy(buff, "");
		read(fd, buff2, 4 * sizeof(char));
		printf("%s\n", buff2);
		read(fd, buff, 2 * sizeof(char));
		if(atoi(buff) >= 35 && atoi(buff) <= 104)
		{
			printf("%d", atoi(buff));
		}
		printf("%s\n", buff);*/
	}
	close(fd);
}

int main(int argc, char **argv)
{
	if(argc >= 2){
        	if(strcmp(argv[1], "variant") == 0){
            		printf("36001\n");
        	}else if(strcmp(argv[1], "list") == 0){
        		int first = 1;
        		int r = 0;
        		off_t sz = 0;
        		char* perm = malloc(512*sizeof(char));
        		for(int i = 2; i < argc - 1; i++){
        			if(strcmp(argv[i], "recursive") == 0)
        				r = 1;
        			else{
        				char* opt = malloc(512*sizeof(char));
        				strncpy(opt, argv[i], 12);
        				if(strcmp(opt, "size_smaller") == 0)
        					sz = atoi(argv[i] + 13);
        				else if(strcmp(opt, "permissions=") == 0){
        					strcpy(perm, argv[i] + 12);
        				}else{ 
        					perror("ERROR\nOptiune invalida!");
        					free(opt);
        					free(perm);
        					return 0;
        				}
        				free(opt);
        			}
        		}
        		list(r, sz, perm, argv[argc-1] + 5, &first);
        		free(perm);
        	}else if(strcmp(argv[1], "parse") == 0){
        		parse(argv[2] + 5);
        	}
    	}
    	return 0;
}
