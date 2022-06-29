# T2-software-basico
Trabalho 2 de software básico PUC-Rio

Nota final: 10/10

Autores:    Gustavo Molina Soares,
            Lucca Vieira Rocha

## Compilador de LBS em C

O objetivo do trabalho é implementar, na linguagem C, uma função (**`gera_codigo`**) que traduz um código na linguagem LBS para Assembly e é compilado e seu resultado mostrado na saída padrão (terminal).

## Sintaxe em LBS

A sintaxe da linguagem LBS pode ser definida formalmente como abaixo. Note que as cadeias entre ' ' são símbolos terminais da linguagem: os caracteres ' não aparecem nos comandos LBS.

pgm	::=	func | func pgm

func	::=	header cmds endf

header	::=	'function\n'

endf	::=	'end\n'

cmds	::=	cmd'\n' | cmd '\n' cmds

cmd	::=	att | ret | zret

att	::=	var '=' expr

expr	::=	oper | call

oper	::=	varpc op varpc

call	::=	'call' num varpc

ret	::=	'ret' varpc

zret	::=	'zret' varpc varpc

var	::=	'v' num

varpc	::=	var | 'p0' | '$' snum

op	::=	'+' | '-' | '*'

num	::=	digito | digito num

snum	::=	[-] num

digito	::=	0' | '1' | '2' | '3' | '4' | '5' | '6' | '7'| '8' | '9'



## Função `gera_codigo`

```c
void gera_codigo(FILE *f, unsigned char code[], funcp *entry);
```

A função **`gera_codigo`** recebe:

- `FILE * f`: O parâmetro f é o descritor de um arquivo texto, já aberto para leitura, de onde deve ser lido o código fonte LBS. 
- `code[]`: O parâmetro code é um vetor onde deverá ser armazenado o código gerado. 
- `entry`: O parâmetro entry é um ponteiro para uma variável (do tipo "ponteiro para função que recebe inteiro e retorna inteiro") onde deve ser armazenado o endereço da função a ser chamada pelo programa principal. 

O arquivo **`FILE *f`** apresenta um código em LBS como o código abaixo:

OBS: Este código serve para retornar os valores da soma dos quadrados dos números inteiros de 1 até o parâmetro recebido

```
    function
    v0 = p0 * p0
    ret v0
    end
    function
    zret p0 $0
    v0 = p0 - $1
    v1 = call 0 p0
    v0 = call 1 v0
    v0 = v0 + v1
    ret v0
    end

```

Para leitura do arquivo só precisaremos ler um caracter apenas, pois esse é o suficiente para dar a informação necessária:

'f' = function
'z' = retorno condicional
'r' = retorno incondicional
'e' = end
'v' = atribuições, onde pode ser dividido em call ('c) e operações aritméticas ('else')

## Restrições

O arquivo LBS tem algumas restrições:
- A quantidade de linhas máximas é 50.
- O arquivo só pode conter no máximo 5 variavéis

## Como compilar o código

```
gcc -Wall -Wa,--execstack -o coderun gera_codigo.c main.c
./coderun
```
