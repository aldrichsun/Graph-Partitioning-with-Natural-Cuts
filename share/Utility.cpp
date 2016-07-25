#include "Utility.h"

void check_file( FILE* fp, const char* filename ){
	if (fp==NULL) {
		printf("Invalid file '%s'\n",filename);
		exit(0);
	}
}

void check_new( void* np, const char* parts ){
	if(np == NULL){
		printf("Bad malloc '%s'\n", parts);
		exit(0);
	}
}