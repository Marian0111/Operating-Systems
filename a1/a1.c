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
	char magic[10];
	fd = open(filePath, O_RDONLY);
	if(fd == -1){
		printf("ERROR\nFisierul nu a putut fi deschis");
	}else{
		read(fd, magic, 4);
		magic[4]= 0;
		if(strcmp(magic, "vumv") != 0)
		{
			printf("ERROR\nwrong magic");
		}else{
			int size = 0, version = 0;
			read(fd, &size, 2);
			read(fd, &version, 2);
			if(version < 35 || version > 104)
			{
				printf("ERROR\nwrong version");
			}else{
				int nr_sections = 0;
				read(fd, &nr_sections, 1);
				if(nr_sections < 3 || nr_sections > 13)
				{
					printf("ERROR\nwrong sect_nr");
				}else{
					char name[13][10];
					int type[13];
					int size[13];
					for(int i = 0; i < nr_sections; i++)
					{
						char name_aux[10];
						int type_aux;
						int size_aux;
						size_t s = read(fd, name_aux, 9);
						name_aux[s] = 0;
						read(fd, &type_aux, 2);
						lseek(fd, 4, SEEK_CUR);
						read(fd, &size_aux, 4);
						if(type_aux != 11 && type_aux != 58 && type_aux != 76)
						{
							printf("ERROR\nwrong sect_types");
							close(fd);
							return;
						}
						strcpy(name[i], name_aux);
						type[i] = type_aux;
						size[i] = size_aux;
					}
					printf("SUCCESS\n");
					printf("version=%d\n", version);
					printf("nr_sections=%d\n", nr_sections);
					for(int i = 0; i < nr_sections; i++)
					{
						printf("section%d: %s %d %d\n", i+1, name[i], type[i], size[i]);
					}
				}
			}
		}
		
	}
	close(fd);
}

void extract(const char *filePath, int section, int line)
{
	int fd = -1;
	fd = open(filePath, O_RDONLY);
	if(fd == -1){
		printf("ERROR\ninvalid file");
	}else{
		int nr_sections = 0;
		lseek(fd, 8, SEEK_CUR);
		read(fd, &nr_sections, 1);
		if(nr_sections < section)
		{
			printf("ERROR\ninvalid section");
		}else{
			for(int i = 1; i < section; i++)
				lseek(fd, 19, SEEK_CUR);
			lseek(fd, 11, SEEK_CUR);
			int offset = 0, size = 0;
			read(fd, &offset, 4);
			read(fd, &size, 4);
			int nr_linie = 1;
			int nr_caractere = 0;
			lseek(fd, offset, SEEK_SET);
			while(nr_caractere < size)
			{
				char b;
				read(fd, &b, 1);
				nr_caractere++;
				if(b == '\n')
					nr_linie++;
			}
			if(nr_linie < line)
			{
				printf("ERROR\ninvalid line");
			}else{
				printf("SUCCESS\n");
				lseek(fd, offset, SEEK_SET);
				int linie_afisare = nr_linie - line + 1;
				nr_linie = 1;
				int nr_caractere = 0;
				while(nr_linie < linie_afisare)
				{
					char b;
					read(fd, &b, 1);
					nr_caractere++;
					if(b == '\n')
						nr_linie++;
				}
				while(nr_caractere < size)
				{
					char b;
					read(fd, &b, 1);
					nr_caractere++;
					if(b != '\n')
						printf("%c", b);
					else{
						break;
					}
				}
			}
		}
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
        	}else if(strcmp(argv[1], "extract") == 0){
        		extract(argv[2] + 5, atoi(argv[3] + 8), atoi(argv[4] + 5));
        	}
    	}
    	return 0;
}
