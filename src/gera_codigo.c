#include <stdio.h>
#include <stdlib.h>

#define LINHAS 50

/* definindo os tamanhos das dos codigos hex para usar depois no codigo sem ter que criar variaveis globais (dica do monitor) */

#define SIZE_FUNCTION 11
#define SIZE_END 2
#define SIZE_RET_CST 5
#define SIZE_RET_PAR 3
#define SIZE_RET_VAR 3
#define SIZE_MOV_CST_REG 6
#define SIZE_MOV_VAR_REG 4
#define SIZE_MOV_PAR_REG 4
#define SIZE_BASE_OPR_CST 7
#define SIZE_BASE_OPR_VAR 4
#define SIZE_BASE_OPR_PAR 4
#define SIZE_MULT_OPR_CST 7
#define SIZE_MULT_OPR_VAR 5
#define SIZE_MULT_OPR_PAR 5
#define SIZE_MOV_CST_PAR 5
#define SIZE_MOV_REG_VAR 4
#define SIZE_MOV_VAR_PAR 3
#define SIZE_MOV_PAR_PAR 3
#define SIZE_CALL 8
#define SIZE_CMPL 4
#define SIZE_JNE 2

/* Prototipos das funções */

typedef int(*funcp)(int x);

void gera_codigo(FILE *f, unsigned char code[], funcp *entry);

static void error (const char *msg, int line);

void lbs_to_asm_func(int * current_byte, unsigned char code[]);

void lbs_to_asm_end(int * current_byte, unsigned char code[]);

void lbs_to_asm_ret(char var, int idx, int * current_byte, unsigned char code[]);

void lbs_to_asm_zret(char var0, char var1, int idx0, int idx1, int * current_byte, unsigned char code[]);

void lbs_to_asm_call(char var0, int idx0, int fx, char var1, int idx1, int * current_byte, unsigned char code[], int func_pos[]);

void lbs_to_asm_opr(char var0, int idx0, char var1, int idx1, char op, char var2, int idx2, int * current_byte, unsigned char code[]);

void inverte_endian(unsigned char *commands, size_t pos, size_t bytes, int number );

void add_commands(unsigned char *commands, size_t bytes, int * current_byte, unsigned char code[]);

/* variavel para debugar 0 == off, 1 == on */

int view_x86_sintax = 0; // variavel global utilizada para ativar o modo print, caso queira ver o código em assembly, isso foi utilizado para auxiliar na hora da construção do código

/* hex commands in assembly */

static unsigned char code_func[SIZE_FUNCTION] = {0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0xec, 0x20, 0x89, 0x7d, 0xe4};
static unsigned char code_end[SIZE_END] = {0xc9, 0xc3};
static unsigned char code_ret_cst[SIZE_RET_CST] = {0xb8, 0x00, 0x00, 0x00, 0x00};
static unsigned char code_ret_par[SIZE_RET_PAR] = {0x8b, 0x45, 0xe4};
static unsigned char code_ret_var[SIZE_RET_VAR] = {0x8b, 0x45, 0x00};
static unsigned char code_mov_cst_reg[SIZE_MOV_CST_REG] = {0x41, 0xba, 0x00, 0x00, 0x00, 0x00};
static unsigned char code_mov_var_reg[SIZE_MOV_VAR_REG] = {0x44, 0x8b, 0x55, 0x00};
static unsigned char code_mov_par_reg[SIZE_MOV_PAR_REG] = {0x44, 0x8b, 0x55, 0xe4};
static unsigned char code_opr_add_cst[SIZE_BASE_OPR_CST] = {0x41, 0x81, 0xc2, 0x00, 0x00, 0x00, 0x00};
static unsigned char code_opr_add_var[SIZE_BASE_OPR_VAR] = {0x44, 0x03, 0x55, 0x00};
static unsigned char code_opr_add_par[SIZE_BASE_OPR_PAR] = {0x44, 0x03, 0x55, 0xe4};
static unsigned char code_opr_sub_cst[SIZE_BASE_OPR_CST] = {0x41, 0x81, 0xea, 0x00, 0x00, 0x00, 0x00};
static unsigned char code_opr_sub_var[SIZE_BASE_OPR_VAR] = {0x44, 0x2b, 0x55, 0x00};
static unsigned char code_opr_sub_par[SIZE_BASE_OPR_PAR] = {0x44, 0x2b, 0x55, 0xe4};
static unsigned char code_opr_mult_cst[SIZE_MULT_OPR_CST] = {0x45, 0x69, 0xd2, 0x00, 0x00, 0x00, 0x00};
static unsigned char code_opr_mult_var[SIZE_MULT_OPR_VAR] = {0x44, 0x0f, 0xaf, 0x55, 0x00};
static unsigned char code_opr_mult_par[SIZE_MULT_OPR_PAR] = {0x44, 0x0f, 0xaf, 0x55, 0xe4};
static unsigned char code_mov_reg_var[SIZE_MOV_REG_VAR] = {0x44, 0x89, 0x55, 0x00};
static unsigned char code_mov_cst_par[SIZE_MOV_CST_PAR] = {0xbf, 0x00, 0x00, 0x00, 0x00};
static unsigned char code_mov_var_par[SIZE_MOV_VAR_PAR] = {0x8b, 0x7d, 0x00};
static unsigned char code_mov_par_par[SIZE_MOV_PAR_PAR] = {0x8b, 0x7d, 0xe4};
static unsigned char code_call[SIZE_CALL] = {0xe8,0x00,0x00,0x00,0x00,0x89,0x45,0x00};
static unsigned char code_zret_cmpl[SIZE_CMPL] = {0x41, 0x83, 0xfa, 0x00};
static unsigned char code_zret_jne[SIZE_JNE] = {0x75, 0x07};

/* main function */

void gera_codigo(FILE *f, unsigned char code[], funcp *entry){

	int c;				/* caracter */
	int line_count = 1;	/* linha atual */
	int current_byte = 0;	/* byte atual */
	int func_count = 0;	/* quantidade de funções */
    int func_pos[LINHAS] = {};	/* posição das funções */

	while((c = fgetc(f)) != EOF){

		switch(c){

			case 'f': { 

				/* caso da função */

				char c0;

				if (fscanf(f, "unction%c", &c0) != 1){
					error("Comando inválido", line_count);
				}
                

                func_pos[func_count] = current_byte;
	            func_count++;
				lbs_to_asm_func(&current_byte, code);
				break;
			}

			case 'e': {

				/* caso do end */

				char c0;

				if (fscanf(f, "nd%c", &c0) != 1){
					error("Comando inválido", line_count);
				}

				lbs_to_asm_end(&current_byte, code); 
				break;
			}

			case 'r': {

				/* caso do retorno incondicional */

				int idx0;
				char var0;

				if (fscanf(f, "et %c%d", &var0, &idx0) != 2){
					error("Comando inválido", line_count);
				}

				lbs_to_asm_ret(var0,idx0, &current_byte, code);
				break;
			}

			case 'z': {

				/* caso do retorno condicional */

				int idx0, idx1;
				char var0, var1;

				if (fscanf(f, "ret %c%d %c%d", &var0, &idx0, &var1, &idx1) != 4){
					error("Comando inválido", line_count);
				}


				lbs_to_asm_zret(var0, var1, idx0, idx1, &current_byte, code);
				break;
			}

			case 'v': {

				/* caso de atribuição */

				char var0 = c, c0;
				int idx0;

				if (fscanf(f, "%d = %c", &idx0, &c0) != 2){
					error("Comando inválido", line_count);
				}

				if ((c0 == 'c')){

					/* caso call */

					int fx, idx1;
					char var1;

					if (fscanf(f, "all %d %c%d",&fx, &var1, &idx1) != 3){
						error("Comando inválido", line_count);
					}

					lbs_to_asm_call(var0, idx0, fx, var1, idx1, &current_byte, code, func_pos);

				}

				else {

					/* caso de operação */

					int idx1, idx2;
					char var1 = c0, var2, op;

					if (fscanf(f, "%d %c %c%d", &idx1, &op, &var2, &idx2) != 4){
						error("Comando inválido", line_count);
					}

					lbs_to_asm_opr(var0, idx0, var1, idx1, op, var2, idx2, &current_byte, code);

				}
				break;
			}
			default: error("Comando desconhecido",line_count);
		}
		line_count++;
		fscanf(f, " ");
	}

	*entry = (funcp) (code + func_pos[func_count - 1]);
}

/* função de erro */

static void error (const char *msg, int line) {
    fprintf(stderr, "Erro %s na linha %d\n", msg, line);
    exit(EXIT_FAILURE);
}


/* funções de lbs para assembly */

void lbs_to_asm_end(int * current_byte, unsigned char code[]){

	/* ------- CASO END ------

	leave			   === LEAVE	0xc9
	ret				   === RETORNO	0xc3

	--------------------------*/
	/*debug assembly code */
	if(view_x86_sintax == 1){
		printf("leave\n");
		printf("ret\n");
		puts("");
	}

	add_commands(code_end, SIZE_END, current_byte, code);
}

void lbs_to_asm_func(int * current_byte, unsigned char code[]){

	/* ------- CASO FUNCTION ------

	pushq %rbp			  ===  INICIALIZANDO A PILHA
	movq %rsp, %rbp		  ===  INICIALIZANDO A PILHA
	subq $32, %rsp		  ===  ABRINDO OS 32 ESPAÇOS PARA ARMAZENAR OS 20 BYTES POSSÍVEIS
	movl %edi, -28(%rbp)  ===  SALVANDO O VALOR DE EDI NA POSIÇÃO 28 PARA PODER ACESSAR ELE DEPOIS

	TRADUÇÃO PARA HEX:
	0:   55                      push   %rbp
    1:   48 89 e5                mov    %rsp,%rbp
    4:   48 83 ec 20             sub    $0x20,%rsp
    8:   89 7d e4                mov    %edi,-0x1c(%rbp)

	--------------------------*/
	if(view_x86_sintax == 1){
		printf("label:\n");
		printf("pushq %%rbp\n");
		printf("movq %%rsp, %%rbp\n");
		printf("subq $32, %%rsp\n");
		printf("movl %%edi, -28(%%rbp)\n");
		puts("");
	}
	add_commands(code_func, SIZE_FUNCTION, current_byte, code);
}

void lbs_to_asm_ret(char var0, int idx0, int * current_byte, unsigned char code[]){

	/* ------- CASO RETORNO INCONDICIONAL ------

	movl varpc, %eax

	------------------------------------------*/

	switch(var0){

		case '$': {
			if(view_x86_sintax){
				printf("movl $%d, %%eax\n",idx0);
			}
			inverte_endian(code_ret_cst, 1, 4, idx0);
			add_commands(code_ret_cst, SIZE_RET_CST, current_byte, code);
			break;
		}

		case 'v': {
			int access_pilha = (-4*(idx0+1));
			if(view_x86_sintax){
				printf("movl %d(%%rbp), %%eax\n",access_pilha);
				puts("");
			}
			inverte_endian(code_ret_var, 2, 1, access_pilha);
			add_commands(code_ret_var, SIZE_RET_VAR, current_byte, code);
			break;
		}

		case 'p': {
			if(view_x86_sintax){
				printf("movl -28(%%rbp), %%eax\n");
				puts("");
			}
			add_commands(code_ret_par, SIZE_RET_PAR, current_byte, code);
			break;
		}
		default: printf("Erro na leitura");  //error


	}
}

void lbs_to_asm_opr(char var0, int idx0, char var1, int idx1, char op, char var2, int idx2, int * current_byte, unsigned char code[]){

	/* ------- CASO OPERAÇÃO -----------------

	movl varpc, %r10d
	addl varpc2, %r10d

	------------------------------------------*/
	switch(var1){
		
		case '$': {
			if(view_x86_sintax){
				printf("movl $%d, %%r10d\n", idx1);
			}
			/*	movl cst, r10d --> 41 ba 00 00 00 00 */
	
			inverte_endian(code_mov_cst_reg, 2, 4, idx1);
			add_commands(code_mov_cst_reg, SIZE_MOV_CST_REG, current_byte, code);
			break;
		}

		case 'v': {
			int access_pilha = -4*(idx1+1);
			if(view_x86_sintax){
				printf("movl %d(%%rbp), %%r10d\n", access_pilha);
			}
			/* 	movl -x(rbp), r10d --> 44 8b 55 00 [obs: x = acess pilha] */

			inverte_endian(code_mov_var_reg, 3, 1, access_pilha);
			add_commands(code_mov_var_reg, SIZE_MOV_VAR_REG, current_byte, code);
			break;
		}

		case 'p': {
			if(view_x86_sintax){
				printf("movl -28(%%rbp), %%r10d\n");
			}
			/*	movl -28(rbp), r10d --> 44 8b 55 e4 */
			add_commands(code_mov_par_reg, SIZE_MOV_PAR_REG, current_byte, code);
			break;
		}
		default: printf("Erro na leitura");  //error
	}

	switch(op){

		case '+': {

			switch(var2){
				
				case '$': {
					if(view_x86_sintax){
						printf("addl $%d, %%r10d\n", idx2);
					}
					/*	addl cst, r10d --> 41 81 c2 00 00 00 00 */
					inverte_endian(code_opr_add_cst, 3, 4, idx2);
					add_commands(code_opr_add_cst, SIZE_BASE_OPR_CST, current_byte, code);
					break;
				}

				case 'v': {
					int access_pilha = -4*(idx2+1);
					if(view_x86_sintax){
						printf("addl %d(%%rbp), %%r10d\n", access_pilha);
					}
					/*	addl -x(rbp), r10d --> 44 03 55 00 */
					inverte_endian(code_opr_add_var, 3, 1, access_pilha);
					add_commands(code_opr_add_var, SIZE_BASE_OPR_VAR, current_byte, code);
					break;
				}

				case 'p': {
					if(view_x86_sintax){
						printf("addl -28(%%rbp), %%r10d\n");
					}
					/*	addl -28(rbp), r10d --> 44 03 55 e3 */
					add_commands(code_opr_add_par, SIZE_BASE_OPR_PAR, current_byte, code);
					break;
				}
				default: printf("Erro na leitura");  //error
			}

			break;
		}

		case '-': {

			switch(var2){
				
				case '$': {
					if(view_x86_sintax){
						printf("subl $%d, %%r10d\n", idx2);
					}
					/*	subl cst, r10d --> 41 83 ea 00 00 00 00 */
					inverte_endian(code_opr_sub_cst, 3, 4, idx2);
					add_commands(code_opr_sub_cst, SIZE_BASE_OPR_CST, current_byte, code);
					break;
				}

				case 'v': {
					int access_pilha = -4*(idx2+1);
					if(view_x86_sintax){
						printf("subl %d(%%rbp), %%r10d\n", access_pilha);
					}
					/* subl -x(rbp), r10d --> 44 2b 55 00 */
					inverte_endian(code_opr_sub_var, 3, 1, access_pilha);
					add_commands(code_opr_sub_var, SIZE_BASE_OPR_VAR, current_byte, code);
					break;
				}

				case 'p': {
					if(view_x86_sintax){
						printf("subl -28(%%rbp), %%r10d\n");
					}
					/*	subl -28(rbp), r10d --> 44 2b 55 e4 */
					add_commands(code_opr_sub_par, SIZE_BASE_OPR_PAR, current_byte, code);
					break;
				}
				default: printf("Erro na leitura");  //error
			}

			break;
		}

		case '*': {

			switch(var2){
				
				case '$': {
					if(view_x86_sintax){
						printf("imull $%d, %%r10d\n", idx2);
					}
					/*	imull cst, r10d --> 45 69 d2 00 00 00 00 */
					inverte_endian(code_opr_mult_cst, 3, 4, idx2);
					add_commands(code_opr_mult_cst, SIZE_MULT_OPR_CST, current_byte, code);
					break;
				}

				case 'v': {
					int access_pilha = -4*(idx2+1);
					if(view_x86_sintax){
						printf("imull %d(%%rbp), %%r10d\n", access_pilha);
					}
					/*	imull -x(rbp), r10d --> 44 0f af 55 00 */
					inverte_endian(code_opr_mult_var, 4, 1, access_pilha);
					add_commands(code_opr_mult_var, SIZE_MULT_OPR_VAR, current_byte, code);
					break;
				}

				case 'p': {
					if(view_x86_sintax){
						printf("imull -28(%%rbp), %%r10d\n");
					}
					/*	imull -28(rbp), r10d --> 44 0f af 55 e4 */
					add_commands(code_opr_mult_par, SIZE_MULT_OPR_PAR, current_byte, code);
					break;
				}
				default: printf("Erro na leitura");  //error
			}
			break;
		}
		default: printf("Erro na leitura");  //error
	}
	int access_pilha = -4*(idx0+1);
	if(view_x86_sintax){
		printf("movl %%r10d, %d(%%rbp)\n", access_pilha);
		puts("");
	}
	/* movl r10d, -x(%rbp) --> 44 89 55 00|hex */
	inverte_endian(code_mov_reg_var, 3, 1, access_pilha);
	add_commands(code_mov_reg_var, SIZE_MOV_REG_VAR, current_byte, code);
}

void lbs_to_asm_call(char var0, int idx0, int fx, char var1, int idx1, int * current_byte, unsigned char code[], int func_pos[]){

	/* ----------------- CASO CALL ----------------

	movl varpc, %edi
	call <fx>
	movl %eax, -x(%rbp)

	---------------------------------------------*/
	switch(var1){

		case '$': {
			if(view_x86_sintax){
				printf("movl $%d, %%edi\n",idx1);
				puts("");
			}
			/* movl cst, edi --> bf 00 00 00 00 */
			inverte_endian(code_mov_cst_par, 1, 4, idx1);
			add_commands(code_mov_cst_par, SIZE_MOV_CST_PAR, current_byte, code);
			break;
		}
		
		case 'v': {
			int access_pilha = -4*(idx1+1);
			if(view_x86_sintax){
				printf("movl %d(%%rbp), %%edi\n", access_pilha);
				puts("");
			}
			/* movl -x(rbp), edi --> 8b 7d 00 */
			inverte_endian(code_mov_var_par, 2, 1, access_pilha);
			add_commands(code_mov_var_par, SIZE_MOV_VAR_PAR, current_byte, code);
			break;
		}

		case 'p': {
			if(view_x86_sintax){
				printf("movl -28(%%rbp), %%edi\n");
				puts("");
			}
			/* movl -28(rbp), edi --> 8b 7d e4|hex */
			add_commands(code_mov_par_par, SIZE_MOV_PAR_PAR, current_byte, code);
			break;
		}
		default: printf("Erro na leitura");  //error
	}
	int access_pilha = -4*(idx0+1);
	if(view_x86_sintax){
		printf("call %d\n",fx);
		printf("movl %%eax, %d(%%rbp)\n",access_pilha);
		puts("");
	}

	/* find function position */

	int aux = (func_pos[fx] - *current_byte - 5);

	inverte_endian(code_call, 1, 4, aux);
	inverte_endian(code_call, 7, 1, access_pilha);
	add_commands(code_call, SIZE_CALL, current_byte, code);
}

void lbs_to_asm_zret(char var0, char var1, int idx0, int idx1, int * current_byte, unsigned char code[]){

	/* ----------------- CASO ZRET (RETORNO CONDICIONAL) ----------------

	movl varpc, r10d
	cmpl $0, %r10d
	movl <retorno>, %eax
	jne <fx>
	leave
	ret

	---------------------------------------------------------------------*/

	switch(var0){

		case '$': {
			if(view_x86_sintax){
				printf("movl $%d, %%r10d\n", idx0);
				puts("");
			}

			inverte_endian(code_mov_cst_reg, 2, 4, idx0);
			add_commands(code_mov_cst_reg, SIZE_MOV_CST_REG, current_byte, code);
			break;
		}

		case 'v': {
			int access_pilha = -4*(idx0+1);

			if(view_x86_sintax){
				printf("movl %d(%%rbp), %%r10d\n", access_pilha);
				puts("");
			}

			inverte_endian(code_mov_var_reg, 3, 1, access_pilha);
			add_commands(code_mov_var_reg, SIZE_MOV_VAR_REG, current_byte, code);
			break;
		}

		case 'p': {
			if(view_x86_sintax){
				printf("movl -28(%%rbp), %%r10d\n");
				puts("");
			}

			add_commands(code_mov_par_reg, SIZE_MOV_PAR_REG, current_byte, code);
			break;
		}
		default: printf("Erro na leitura");  //error
	}
	if(view_x86_sintax){

		printf("cmpl $0, %%r10d\n");
		printf("movl %c%d, %%eax\n",var1,idx1);
		printf("jne label\n");

	}

	add_commands(code_zret_cmpl, SIZE_CMPL, current_byte, code);

	add_commands(code_zret_jne, SIZE_JNE, current_byte, code);

	lbs_to_asm_ret(var1, idx1, current_byte, code);

	lbs_to_asm_end(current_byte, code);
}


/* insere o numero em little endian em hex no codigo */

void inverte_endian(unsigned char *commands, size_t hex_pos, size_t hex_bytes, int num){

	int i = 0;
	char byte_zero = 0x00;
	char num_byte;

	if(num < 0){
		num = ~(-(num+1));
		byte_zero = 0xff;
	}

	for (int j = 0; j < hex_bytes; j++){
		num_byte = num & 0xff;
		commands[hex_pos+i] = num ? num_byte : byte_zero;
		num = num >> 8;
		i += 1;
	}
	
}

/* adiciona comandos ao array */

void add_commands(unsigned char *commands, size_t bytes, int * current_byte, unsigned char code[]){
	for (int i = 0; i < bytes; i++) {
		code[*current_byte] = commands[i];
		*current_byte += 1;
	}
}