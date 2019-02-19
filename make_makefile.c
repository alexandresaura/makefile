#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1000
#define PROJECT_NAME "project"
#define MAIN_FILE "main.c"
#define MAKEFILE_NAME "makefile"
#define MAKEFILE_TMP "makefile_tmp"
#define TRUE 1
#define FALSE 0

FILE * makefile_tmp;

int search_dependencies(char *);
int write_dependencies(char *, int, char [][MAX_SIZE]);
int create_makefile(void);

int main() {
    //Cria o arquivo makefile temporário
    makefile_tmp = fopen(MAKEFILE_TMP, "w");
    if(makefile_tmp == NULL) return 1;

    //Escreve as dependencias no makefile temporário, começando pelo arquivo MAIN_FILE
    if(!search_dependencies(MAIN_FILE)) return 1;
    fclose(makefile_tmp);

    //Muda o makefile temporário para leitura
    makefile_tmp = fopen(MAKEFILE_TMP, "r");
    if(makefile_tmp == NULL) return 1;

    //Cria o makefile
    if(!create_makefile()) return 1;

    //Fecha e remove o makefile temporário
    fclose(makefile_tmp);
    remove(MAKEFILE_TMP);

    return 0;
}

int search_dependencies(char * file_name) {

    //Abre o arquivo
    FILE * file = fopen(file_name, "r");
    if(file == NULL) return FALSE;

    char line[MAX_SIZE];

    //Conta quantas denpendencias há no arquivo e retorna o ponteiro para o início
    int count_dependencies = 0;
    while(fgets(line, MAX_SIZE, file) != NULL) {

        //Verifica se a a linha há '#include "' ou '#include"'
        if(strstr(line, "#include \"") || strstr(line, "#include\"")) {
            count_dependencies ++;
        }
    }
    rewind(file);

    //Cria matriz que vai armazenar as dependências
    char dependencies_h[count_dependencies][MAX_SIZE];

    char * dependency_extension_h;
    char dependency_extension_c[MAX_SIZE];

    //Percorre novamento o arquivo agora atribuindo as dependências na matriz criada e retorna o ponteiro para o início
    count_dependencies = 0;
    while (fgets(line, MAX_SIZE, file) != NULL) {

        //Verifica se a a linha há '#include "' ou '#include"'
        if(strstr(line, "#include \"") || strstr(line, "#include\"")) {

            //Da split na linha do "include" para pegar apenas o nome da denpendência
            strtok(line, "\"");
            dependency_extension_h = strtok(NULL, "\"");

            //Transforma a dependência ".h" em ".c"
            strcpy(dependency_extension_c, dependency_extension_h);
            int depedency_size = strlen(dependency_extension_c);
            dependency_extension_c[depedency_size - 1] = 'c';

            //Se a dependência original, com a extensão ".c", não tem o mesmo nome do arquivo
            //Procura as depências da dependência original do ".c"
            if(strcmp(file_name, dependency_extension_c) != 0) {
                search_dependencies(dependency_extension_c);
            }

            //Atribui a depêndencia na matriz
            strcpy(dependencies_h[count_dependencies++], dependency_extension_h);
        }
    }
    rewind(file);

    //Escreve a dependência no makefile temporário
    if(!write_dependencies(file_name, count_dependencies, dependencies_h)) return FALSE;

    return TRUE;
}

int write_dependencies(char * file_extension_c, int count_dependencies, char dependencies_h[count_dependencies][MAX_SIZE]) {
    
    //Transforma a dependência ".c" em ".o"
    char file_extension_o[MAX_SIZE];
    strcpy(file_extension_o, file_extension_c);
    int file_size = strlen(file_extension_o);
    file_extension_o[file_size - 1] = 'o';

    //Aponta para o final do arquivo
    fseek(makefile_tmp, 0, SEEK_END);

    //Escreve as dependências para gerar o ".o" e retorna o ponteiro para o início
    fprintf(makefile_tmp, "\n%s: %s", file_extension_o, file_extension_c);
    for(int i = 0; i < count_dependencies; i++) {
        fprintf(makefile_tmp, " %s", dependencies_h[i]);
    }
    fprintf(makefile_tmp, "\n");
    fprintf(makefile_tmp, "\tgcc -c %s\n", file_extension_c);
    rewind(makefile_tmp);

    return TRUE;
}

int create_makefile() {
    //Cria o arquivo makefile
    FILE * makefile = fopen(MAKEFILE_NAME, "w");
    if(makefile == NULL) return FALSE;

    //Escreve o começo da primeira linha, com o nome do projeto
    fprintf(makefile, "%s:", PROJECT_NAME);

    char line[MAX_SIZE];
    char * dependency_extension_o;

    //Buscas as dependências ".o" no makefile temporário
    //Escreve as dependências ".o" na primeira linha do makefile
    //Retorna o ponteiro para o início do makefile temporário
    while(fgets(line, MAX_SIZE, makefile_tmp) != NULL) {

        //Se há um ":" pega o nome do arquivo no makefile temporário e insere no makefile
        if(strstr(line, ":")) {
            dependency_extension_o = strtok(line, ":");
            fprintf(makefile, " %s", dependency_extension_o);
        }
    }
    rewind(makefile_tmp);

    //Escreve o começo da segunda linha, com o nome do projeto
    fprintf(makefile, "\n");
    fprintf(makefile, "\tgcc -o %s", PROJECT_NAME);

    //Buscas as dependências ".o" no makefile temporário
    //Escreve as dependências ".o" na segunda linha do makefile
    //Retorna o ponteiro para o início do makefile temporário
    while(fgets(line, MAX_SIZE, makefile_tmp) != NULL) {
        if(strstr(line, ":")) {
            dependency_extension_o = strtok(line, ":");
            fprintf(makefile, " %s", dependency_extension_o);
        }
    }
    fprintf(makefile, " -lm");
    fprintf(makefile, "\n");
    rewind(makefile_tmp);

    //Copia o makefile temporário para o makefile e fecha o makefile
    while(fgets(line, MAX_SIZE, makefile_tmp) != NULL) {
        fprintf(makefile, "%s", line);
    }
    fclose(makefile);

    return TRUE;
}