#include <stdio.h>
#include "gera_codigo.h"

int main(int argc, char *argv[]){
	FILE *fp;
	unsigned char code[1600];
	funcp funcLBS;
	int res;

	fp = fopen("LBS_factorial.txt","r");

	gera_codigo(fp,code,&funcLBS);
	if (funcLBS == NULL){
		printf("Erro na geração\n");
	}

	res = (*funcLBS)(3);
	printf("%d", res);
	fclose(fp);
	return 0;
}