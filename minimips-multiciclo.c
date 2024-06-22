#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {tipo_R = 0, tipo_I = 1, tipo_J = 2} Tipo_inst;

typedef struct {
    Tipo_inst tipo_inst;
    char inst_char[18];
    char dados_char[10];
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
    int dados;
} mem;

typedef struct No {
    int reg_backup[8];
    int pc_backup;
    int dados_backup[256];
	int clock;
	int indice;
    struct No *prox;
    struct No *ant;
} No;

typedef struct _backup{
	No *primeiro;
	No *ultimo;
	int tamanho;
}Backup;


int binario_para_decimal_c2(const char *binario, int bits);
int preencher_memoria_inst(FILE *file_mem, mem *memoria, int linhas_mem);
char *busca(mem *memoria, int pc);
int binario_para_decimal(const char *binario);
int ula(mem memoria, int *registradores, int *i, int *flag, int somapc);
void overflow(mem memoria, int *registradores, int *flag);
void decodificacao(mem *memoria_inst, char *instrucao_buscada, int pc);
int controle(mem *instrucoes, int linhas_mem, int *registradores, int *pc, int *flag);
void gerencia_escrita(mem *instrucoes, int pc, int escrita, int *registradores, int *MDR);
void imprimeregistradores(int *registradores);
void imprimeMemoria(mem *memoria);
void imprime_decodificacao(mem *memoria_inst, int i);
void salva_estado_memoria(mem *memoria);
void salva_asm(mem *memoria_inst, int j);
void imprimeInstrucoes(mem *memoria_inst, int linhas_mem);
void imprime_asm(mem *memoria_inst, int j);
void decimalParaBinario(int valor, mem *memoria, int indice);


No* criaNo(int *registradores, int pc, mem *memoria, int clock, int indice);
void insereNo(Backup *backup, No *no);
Backup *criaBackup();
void restauraUltimoBackup(Backup *backup, int *registradores, int *pc, mem *memoria, int *clock, int *indice);



int main(){
    int registradores[8] = {0};
	mem memoria[256];
	FILE *file_mem;
    int linhas_mem = 0;
    char barraN;
    int menu;
    int pc = 0; // Inicializa pc com zero
    int flag=0;
	char instrucao_buscada[18];
    int escrita = 0;
    int MDR;
    int clock=0;  //inicia em 0 e sofre o primeiro incremento quando iniciamos o simulador
    int n_inst = 0;

    Backup *backup = criaBackup();
    No *no = NULL;

    //variaveis pra verificao
    //usadas no clock
    char opcode_temp[6];
    int opcode, indice=0;   
    
    flag = 0;
    
    for (flag=0; flag<256; flag++)
    	memoria[flag].dados = 0;
    
    do {
        printf("\n\n========================= MENU MINIMIPS MULTICICLO ========================\n");
        printf("\nSelecione uma opcao:\n"
                "(1) Para iniciar o simulador\n"
                "(2) Clock\n"
                "(3) Para imprimir a memoria\n" 
                "(4) Para imprimir registradores\n"
                "(5) Para imprimir todo o simulador\n"
                "(6) Para salvar estado da memoria\n"
                "(8) Para salvar as instruções decodificadas em nível de montagem\n"
                "(9) Para voltar um ciclo\n"
			    "(0) Para sair\n");
	    printf("\n===========================================================================\n");
        scanf("%d", &menu);

        switch (menu) {
           
            case 1:
                if ((file_mem = fopen("multiplicacao.mem", "r")) == NULL) {
                    puts("Erro: memoria nao foi carregada.");
                    return 1;
                }

                while ((barraN = fgetc(file_mem)) != EOF) {
                    if (barraN == '\n')
                        linhas_mem++;
                }

                rewind(file_mem);
                n_inst = preencher_memoria_inst(file_mem, memoria, linhas_mem);
                fclose(file_mem);
                clock++;    //a ideia do primeiro incremento do clock ser aqui é poder testar se o simulador já foi iniciado
                break;

            case 2: //inicia o clock
              
                switch (clock)  //vai testar o clock
                {
                case 0:
                    printf("\nVocê deve iniciar o simulador\n");
                    break;

                case 1:   //busca - estagio 0, todos devem passar por esse estagio
                    // ############## O USUARIO PRECISA VER QUE EM EM CASO DE DESVIO O PC É DIFERENTE DE PC + 1 ###############
                    if(indice<n_inst && n_inst != 0 && strlen(memoria[indice].inst_char) == 17)
                    {
                            no = criaNo(registradores, pc, memoria, clock, indice);
						    insereNo(backup, no);
						    
                            indice = pc;
                            
                            strcpy(instrucao_buscada, busca(memoria, pc));
                            
                            printf("\n===========================================================================\n");
                            printf("\nClock %i -> BUSCA\n", clock);
                            printf("A instrução buscada da memoria é: %s", instrucao_buscada);
                            
                            strncpy(opcode_temp, instrucao_buscada, 4);
                            opcode_temp[4] = '\0';
                            opcode = binario_para_decimal(opcode_temp);
                            
                                if (opcode == 2 || opcode ==8){
                                    printf("Pc = Pc + desvio\n");
                                }
                                else{
                                    ula(memoria[pc], registradores, &pc, &flag, 1); // incrementa PC
                                    printf("PC = PC + 1\n");
                                    printf("PC = %i\n", pc);
                                }
                            printf("\n===========================================================================\n");
                            clock++;
                    }
                    else{
                        printf("\nAVISO: Não há instruções a serem buscadas!\n");
                        clock=1;
                    }
                        
                        
                    break;
                    
                   case 2:  //decodificacao - estagio 1, todos devem passar por esse estagio 
                        no = criaNo(registradores, pc, memoria, clock, indice);
						insereNo(backup, no);
						
                        decodificacao(memoria, instrucao_buscada, indice);
                        printf("\n===========================================================================\n");
                        printf("\nClock %i -> DECODIFICAÇÃO\n", clock);
                        printf("A instrução decodificada é: ");
                        imprime_decodificacao(memoria, indice);
                        printf("No total, esta instrução demanda ");
                        switch(memoria[indice].opcode)
                        {
                            case 0:
                                printf("4 ciclos\n");
                                break;
                            case 4:
                                printf("4 ciclos\n");
                                break;
                            case 15:
                                printf("4 ciclos\n");
                                break;
                            case 11:
                                printf("5 ciclos\n");
                                break;
                            case 8:
                                printf("3 ciclos\n");
                                break;
                            case 2:
                                printf("3 ciclos\n");
                                break;
                        }   
                        printf("\n===========================================================================\n");
                    
                        clock++;
                    break;

                    case 3:  //execucao - estagio 2 (encerram: BEQ E JUMP)
                        no = criaNo(registradores, pc, memoria, clock, indice);
						insereNo(backup, no);
                    
                        printf("\n===========================================================================\n");
                        printf("\nClock %i -> ", clock);
                        
                        if(memoria[indice].opcode == 0 || memoria[indice].opcode == 4 || memoria[indice].opcode == 15 || memoria[indice].opcode == 11)   
                        	escrita = controle(memoria, linhas_mem, registradores, &indice, &flag);
                        
                        switch(memoria[indice].opcode)
                        {
                            case 0:
                                printf("EXECUÇÃO\nAluOut = %i\n", escrita);
                                break;
                            case 4:
                                printf("EXECUÇÃO\nAluOut = %i\n", escrita);
                                break;
                            case 15:
                                printf("COMPUTAÇÃO DE ENDEREÇO\nAluOut = %i\n", escrita);
                                break;
                            case 11:
                                printf("COMPUTAÇÃO DE ENDEREÇO\nAluOUT = %i\n", escrita);
                                break;
                            case 8:
                            	controle(memoria, linhas_mem, registradores, &indice, &flag);
                                printf("CONCLUSÃO DO BRANCH\nPC = AluOut = %i\n", indice);
                                break;
                            case 2:
                            	controle(memoria, linhas_mem, registradores, &indice, &flag);
                                printf("CONCLUSÃO DO JUMP\nPC = %i\n", memoria[indice].addr);
                                break;
                        }   
                        printf("\n===========================================================================\n");
                        
                        
                        if (opcode == 2 || opcode == 8){
                            clock=1;
                            pc=indice;
                        }
                        else{
                            clock++;
                        }
                        
                    break;
                    
                    case 4:  // escrita - estagio 3 (encerram: ADD, SW, ADDI)

                        no = criaNo(registradores, pc, memoria, clock, indice);
						insereNo(backup, no);	

                        
                        gerencia_escrita(memoria, indice, escrita, registradores, &MDR);
                        
                        printf("\n===========================================================================\n");
                        printf("\nClock %i -> ", clock);
                        
                        switch(memoria[indice].opcode)
                        {
                            case 0:
                                printf("CONCLUSÃO DA OPERAÇÃO ARITMÉTICA\n[$%i] = %i\n", memoria[indice].rd, registradores[memoria[indice].rd]);
                                break;
                            case 4:
                                printf("CONCLUSÃO DA OPERAÇÃO ARITMÉTICA\n[$%i] = %i\n", memoria[indice].rt, registradores[memoria[indice].rt]);
                                break;
                            case 15:
                                printf("ACESSO À MEMÓRIA\n[%i] = %i\n", memoria[indice].imm, memoria[memoria[indice].imm].dados);
                                break;
                            case 11:
                                printf("ACESSO À MEMÓRIA\nMDR = %i\n", escrita);
                                break;
                        }   
                        printf("\n===========================================================================\n");
                    
                        if (opcode == 15 || opcode == 0 || opcode == 4 ){
                            clock=1;
                            indice=pc;
                        }
                        else{
                            clock++;
                        }
                    break;

                    case 5:  //escreve da memoria no registrador (encerra: LW)
                    
                        no = criaNo(registradores, pc, memoria, clock, indice);
						insereNo(backup, no);
                    
                        registradores[memoria[indice].rt] = MDR;
                        
                        printf("\n===========================================================================\n");
                        printf("\nClock %i -> CONCLUSÃO DA LEITURA DA MEMÓRIA\n", clock);
                        printf("[$%i] = %i\n", memoria[indice].rt, registradores[memoria[indice].rt]);
                        printf("\n===========================================================================\n");
                        clock=1; // recebe 1 para entrar no primeiro case do switch clock
                        indice=pc;
                        break;
    
                    default:
                        break;
                    }
                break;
            case 3:
                imprimeMemoria(memoria);
                break;

            case 4:
                imprimeregistradores(registradores);
                break;

            case 5:
                imprimeInstrucoes(memoria, linhas_mem);
		        imprime_asm(memoria, pc);
                imprimeregistradores(registradores);
                printf("\nFlag = %d", flag);
		        printf("\n\nPC = %d\n\n", pc);
				break;

            case 6:
                salva_estado_memoria(memoria);
                break;
		
            case 8:
                salva_asm(memoria, pc);
                break;
                 
            case 9:
		restauraUltimoBackup(backup, registradores, &pc, memoria, &clock, &indice);
		break;  
            }
        }while (menu != 0);
             


return 0;
}

int preencher_memoria_inst(FILE *file_mem, mem *memoria, int linhas_mem) {
    char temporario[18];
    int inst = 0;
    for (int i = 0; i < linhas_mem; i++)
        {
            fgets(temporario, sizeof(temporario), file_mem);
            if(strlen(temporario)<=10)
            {
                strcpy(memoria[i].dados_char, temporario);
                memoria[i].dados = binario_para_decimal_c2(memoria[i].dados_char, 8);
            }
            else
            {
                strcpy(memoria[i].inst_char, temporario);
                inst++;
            }
        }
    return inst;
}        

int binario_para_decimal_c2(const char *binario, int bits) {
    int decimal = 0;
    int sinal = 0;


    if (binario[0] == '1') {
        sinal = 1;
    }

    for (int i = 0; i < bits; i++) {
        decimal = decimal * 2 + (binario[i] - '0');
    }


    if (sinal) {
        decimal -= (1 << bits);
    }

    return decimal;
}


char *busca(mem *memoria, int pc)
{
    return memoria[pc].inst_char;
}

int ula(mem memoria, int *registradores, int *pc, int *flag, int somapc){
	if(somapc == 0)
	{
		switch(memoria.opcode)
		{
			case 0:		
				switch(memoria.funct)
				{
					case 0: // add
						overflow(memoria, registradores, flag);
						return registradores[memoria.rs] + registradores[memoria.rt];
						break;
					case 2: // sub
						overflow(memoria, registradores, flag);
						return registradores[memoria.rs] - registradores[memoria.rt];
						break;
					case 4: // and
						overflow(memoria, registradores, flag);
						return registradores[memoria.rs] & registradores[memoria.rt];
						break;
					case 5: // or
						overflow(memoria, registradores, flag);
						return registradores[memoria.rs] + registradores[memoria.rt];
						break;
				}
				break;
			case 4://addi
				overflow(memoria, registradores, flag);
						return registradores[memoria.rs] + memoria.imm;
				break;
			case 8://beq
				if (registradores[memoria.rs] == registradores[memoria.rt]){
                    			*pc = (*pc + 1) + memoria.imm;
                }
                    else{
                        *pc = *pc + 1;
                    }
				break;
			case 2://jump
				  *pc = memoria.addr;
				break;
		}
	}
	else
		*pc = *pc + 1;
}

void overflow(mem memoria, int *registradores, int *flag){

    
    if (memoria.imm < -32 || memoria.imm > 31 || registradores[memoria.rt] < -128 || registradores[memoria.rt] > 127){
        printf("\nImpossível realizar operação! Overflow!");
        registradores[memoria.rt]=0;
        *flag=1;
    }
    else{
         if (registradores[memoria.rd] < -128 || registradores[memoria.rd] > 127){
        registradores[memoria.rd]=0;
        printf("\nImpossível realizar operação! Overflow!");
        *flag=1;
        }

    }
}

int binario_para_decimal(const char *binario) {
    int decimal = 0;
    while (*binario != '\0') {
        decimal = decimal * 2 + (*binario - '0');
        binario++;
    }
    return decimal;
}


void decodificacao(mem *memoria_inst, char *instrucao_buscada, int pc) {
    char opcode_temp[6], rs_temp[4], rt_temp[4], rd_temp[4], funct_temp[4], imm_temp[7], addr_temp[8];
   
    strncpy(opcode_temp, instrucao_buscada, 4);
    opcode_temp[4] = '\0';
    memoria_inst[pc].opcode = binario_para_decimal(opcode_temp);

    if (memoria_inst[pc].opcode == 0)
        memoria_inst[pc].tipo_inst = tipo_R;
    else if (memoria_inst[pc].opcode == 4 || memoria_inst[pc].opcode == 11 || memoria_inst[pc].opcode == 15 ||
             memoria_inst[pc].opcode == 8)
        memoria_inst[pc].tipo_inst = tipo_I;
    else if (memoria_inst[pc].opcode == 2)
        memoria_inst[pc].tipo_inst = tipo_J;

    switch (memoria_inst[pc].tipo_inst) {
        case tipo_R:
            strncpy(rs_temp, instrucao_buscada + 4, 3);
            rs_temp[3] = '\0';
            strncpy(rt_temp, instrucao_buscada + 7, 3);
            rt_temp[3] = '\0';
            strncpy(rd_temp, instrucao_buscada + 10, 3);
            rd_temp[3] = '\0';
            strncpy(funct_temp, instrucao_buscada + 13, 3);
            funct_temp[3] = '\0';
            memoria_inst[pc].rs = binario_para_decimal(rs_temp);
            memoria_inst[pc].rt = binario_para_decimal(rt_temp);
            memoria_inst[pc].rd = binario_para_decimal(rd_temp);
            memoria_inst[pc].funct = binario_para_decimal(funct_temp);
            break;

        case tipo_I:
            strncpy(rs_temp, instrucao_buscada + 4, 3);
            rs_temp[3] = '\0';
            strncpy(rt_temp, instrucao_buscada + 7, 3);
            rt_temp[3] = '\0';
            strncpy(imm_temp, instrucao_buscada + 10, 6);
            imm_temp[6] = '\0';
            memoria_inst[pc].rs = binario_para_decimal(rs_temp);
            memoria_inst[pc].rt = binario_para_decimal(rt_temp);
            memoria_inst[pc].imm = binario_para_decimal_c2(imm_temp, 6);
            break;

        case tipo_J:
            strncpy(addr_temp, instrucao_buscada + 9, 7);
            addr_temp[7] = '\0';
            memoria_inst[pc].addr = binario_para_decimal_c2(addr_temp, 7);
            break;
    }
    
}


int controle(mem *instrucoes, int linhas_mem, int *registradores, int *pc, int *flag) {
        switch (instrucoes[*pc].opcode) 
		{
            case 0://Instrução aritmética -> tipo_R
                return ula(instrucoes[*pc], registradores, pc, flag, 0);
                break;
            case 4: //Addi -> tipo_I
                return ula(instrucoes[*pc], registradores, pc, flag, 0);
                break;
            case 15://sw -> tipo_I
                return registradores[instrucoes[*pc].rt];
                break;
            case 11://lw -> tipo_I
                return instrucoes[instrucoes[*pc].imm].dados;
                break;
            case 8://beq -> tipo_I
                ula(instrucoes[*pc], registradores, pc, flag, 0);
                break;
            case 2://j -> tipo_J
		ula(instrucoes[*pc], registradores, pc, flag, 0);
                break;
        }
}


void gerencia_escrita(mem *instrucoes, int pc, int escrita, int *registradores, int *MDR)
{
    switch(instrucoes[pc].opcode)
        {
            case 0: // se for do tipo r transfere pro rd
                registradores[instrucoes[pc].rd] = escrita;
		        break;
            case 4: // se for addi transfere pro rt
                registradores[instrucoes[pc].rt] = escrita;
                break;
            case 15: // se for sw transfere pra respectiva posicao da memória;
                instrucoes[instrucoes[pc].imm].dados = escrita;
                break;
            case 11: // se for lw transfere pro rt
                *MDR = escrita;
                break;
        }
        
}

void imprimeregistradores(int *registradores) {
    puts("\nREGISTRADORES:");
    for (int i = 0; i < 8; i++)
        printf("registrador[%d]: %d\n", i, registradores[i]);
    putchar('\n');
}

void imprimeMemoria(mem *memoria)
{
    int i;
    for(i=0; i<256; i++)
        {
            if(strlen(memoria[i].dados_char) == 9 || memoria[i].dados > 0)
                printf("[%i] %i\n", i, memoria[i].dados);
            else
                if(strlen(memoria[i].inst_char) == 17)
                    printf("[%i] %s", i, memoria[i].inst_char);
                else
                    printf("[%i] 0\n", i);
        }
}

No* criaNo(int *registradores, int pc, mem *memoria, int clock, int indice) {
    No *no = (No *)malloc(sizeof(No));
    for (int i = 0; i < 8; i++) {
        no->reg_backup[i] = registradores[i];
    }
    no->pc_backup = pc;
    for (int i = 0; i < 256; i++) {
        no->dados_backup[i] = memoria[i].dados;
    }
	no->indice = indice;
	no->clock = clock;
    no->prox = NULL;
    no->ant = NULL;
    return no;
}

void insereNo(Backup *backup, No *no) {
    if (backup->tamanho >= 256) {
        No *primeiro = backup->primeiro;
        backup->primeiro = primeiro->prox;
        if (backup->primeiro != NULL) {
            backup->primeiro->ant = NULL;
        } else {
            backup->ultimo = NULL;
        }
        free(primeiro);
    } else {
        if (backup->tamanho == 0) {
            backup->primeiro = no;
            backup->ultimo = no;
        } else {
            no->ant = backup->ultimo;
            backup->ultimo->prox = no;
            backup->ultimo = no;
        }
        backup->tamanho++;
    }
}

Backup *criaBackup(){
	Backup *backup = (Backup *)calloc(1, sizeof(No));
	backup->primeiro = NULL;
	backup->ultimo = NULL;
	backup->tamanho = 0;

	return backup;
}

void restauraUltimoBackup(Backup *backup, int *registradores, int *pc, mem *memoria, int *clock, int *indice) {
    if (backup == NULL || backup->ultimo == NULL) {
        printf("Nenhum backup para restaurar.\n");
        return;
    }

    No *ultimo = backup->ultimo;

    
    for (int i = 0; i < 8; i++) {
        registradores[i] = ultimo->reg_backup[i];
    }

    
    *pc = ultimo->pc_backup;
	*clock = ultimo->clock;
	*indice = ultimo->indice;

    
    for (int i = 0; i < 256; i++) {
        memoria[i].dados = ultimo->dados_backup[i];
    }

    
    if (ultimo->ant != NULL) {
        ultimo->ant->prox = NULL;
    } else {
        
        backup->primeiro = NULL;
    }
    backup->ultimo = ultimo->ant;
    backup->tamanho--;

    free(ultimo);
    printf("Backup restaurado com sucesso.\n");
}

void imprime_decodificacao(mem *memoria_inst, int i) {
    switch (memoria_inst[i].tipo_inst) {
        case tipo_R:
            switch (memoria_inst[i].funct) {
                case 0:
                    printf("add $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
                case 2:
                    printf("sub $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
                case 4:
                    printf("and $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
                case 5:
                    printf("or $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
            }
            break;
        case tipo_I:
            switch (memoria_inst[i].opcode) {
                case 4:
                    printf("addi $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                    break;
                case 11:
                    printf("lw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                    break;
                case 15:
                    printf("sw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                    break;
                case 8:
                    printf("beq $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                    break;
            }
            break;
        case tipo_J:
            printf("J %i\n", memoria_inst[i].addr);
            break;
        
    }
}

void imprimeInstrucoes(mem *memoria_inst, int linhas_mem) {
    puts("\nINSTRUCOES:");
    for (int i = 0; i < linhas_mem; i++)
        if(strlen(memoria_inst[i].inst_char) > 10)
            printf("%s", memoria_inst[i].inst_char);
    putchar('\n');
}

void salva_estado_memoria(mem *memoria) {
    FILE *arquivo;
    int i;
    if ((arquivo = fopen("memoria.mem", "w")) == NULL) {
        printf("Erro na abertura do arquivo");
        return;
    }
    for (i = 0; i < 256; i++) {
        if(strlen(memoria[i].dados_char) == 9 || memoria[i].dados != 0){
        	decimalParaBinario(memoria[i].dados, memoria, i);
                fprintf(arquivo,"%s\n", memoria[i].dados_char);
                }
            else
                if(strlen(memoria[i].inst_char) == 17)
                    fprintf(arquivo, "%s", memoria[i].inst_char);
                else
                    fprintf(arquivo, "00000000\n");
    }
    fclose(arquivo);
}

void imprime_asm(mem *memoria_inst, int j) {
    printf("INSTRUCOES .ASM:\n");
	for (int i = 0; i < j; i++) {
	    if(strlen(memoria_inst[i].inst_char) > 10)
	    {
            switch (memoria_inst[i].tipo_inst) {
                case tipo_R:
                    switch (memoria_inst[i].funct) {
                        case 0:
                            printf("add $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 2:
                            printf("sub $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 4:
                            printf("and $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 5:
                            printf("or $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                    }
                    break;
                case tipo_I:
                    switch (memoria_inst[i].opcode) {
                        case 4:
                            printf("addi $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                        case 11:
                            printf("lw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 15:
                            printf("sw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 8:
                            printf("beq $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                    }
                    break;
                case tipo_J:
                    printf("J %i\n", memoria_inst[i].addr);
                    break;
            }
	    }
    }
}


void salva_asm(mem *memoria_inst, int j) {
    FILE *arquivo;
    if ((arquivo = fopen("ProgramaAssembly.asm", "w")) == NULL) {
        printf("Erro na abertura do arquivo");
        return;
    }
    for (int i = 0; i < j; i++) {
        if(strlen(memoria_inst[i].inst_char) > 10)
	    {
            switch (memoria_inst[i].tipo_inst) {
                case tipo_R:
                    switch (memoria_inst[i].funct) {
                        case 0:
                            fprintf(arquivo, "add $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 2:
                            fprintf(arquivo, "sub $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 4:
                            fprintf(arquivo, "and $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 5:
                            fprintf(arquivo, "or $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                    }
                    break;
                case tipo_I:
                    switch (memoria_inst[i].opcode) {
                        case 4:
                            fprintf(arquivo, "addi $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                        case 11:
                            fprintf(arquivo, "lw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 15:
                            fprintf(arquivo, "sw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 8:
                            fprintf(arquivo, "beq $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                    }
                    break;
                case tipo_J:
                    fprintf(arquivo, "J %i\n", memoria_inst[i].addr);
                    break;
            
            }
        }
    }
    fclose(arquivo);
}

void decimalParaBinario(int valor, mem *memoria, int indice) {
    // Inicializa o buffer com zeros
    memset(memoria[indice].dados_char, '0', 8);
    memoria[indice].dados_char[8] = '\0'; // Adiciona o terminador nulo

    // Converte o valor decimal para binário
    for (int i = 7; i >= 0; i--) {
        memoria[indice].dados_char[i] = (valor % 2) ? '1' : '0';
        valor /= 2;
    }
}

