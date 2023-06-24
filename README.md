# Trabalho prático da disciplina Fundamentos de Sistemas Operacionais

## Escalonador de processos

* O trabalho consiste na implementação de um escalonador simples que irá ter 4 
processos auxiliares 
* Esses processos irão executar processos com tempos de execução diferentes, 
podendo ser de: 5, 10 ou 20 segundos
* Existem dois tipos de modo de execução: o modo normal e o modo roubo de 
trabalho
    * No modo **normal**, quando um processo auxiliar termina de executar todos 
    os seus processos ele avisa o escalonador
    * No modo **roubo de trabalho**, quando um processo auxiliar termina de executar
    todos os seus processos, ele irá executar os arquivos dos outros processos
    que ainda não foram executados

## Implementação

* A implementação foi feita utilizando a linguagem C
* Para realizar a comunicação entre os processos foram utilizados *pipes* e 
*sinais*
* No arquivo *processes.txt* há uma sequencia de números que irão definir o 
tempo de execução
    * 1 -> 5 segundos
    * 2 -> 10 segundos
    * 3 -> 20 segundos
    
## Execução

* Para execução do programa é necessário primeiro compilar o arquivo *main.c*
com o comando `gcc main.c`
* Após a compilação devemos executar o programa `./a.out` 
* Para executar o programa em modo roubo de trabalho deve ser acresecentado a
flag `./a.out --work-stealing`

