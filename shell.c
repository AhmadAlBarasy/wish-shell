#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_ARGS 64
#define CWD_MAX 1024

void execute_command(char* cmd, int is_single);
void read_args(char** token, char** args);
char** extract_commands(char* line, int* commands_num);
void skip_spaces(char** str);

char cwd[CWD_MAX];
char* buff = NULL;
size_t len = 0;
int num_commands = 0;

int main(int argc, char** argv){

    getcwd(cwd, CWD_MAX);

    while(1){
        printf("wish:~%s$ ", cwd);
        ssize_t res = getline(&buff, &len, stdin);

        if(res == -1) break;

        if(res > 0 && buff[res - 1] == '\n'){
            buff[res - 1] = '\0';
        }

        char** commands = extract_commands(buff, &num_commands);

        if(num_commands == 1){
            free(commands);
            execute_command(buff, 1);
        }
        else {
            char** temp_ptr = commands;
            while((*commands) != NULL){
                execute_command(*commands, 0);
                commands++;
            }
            free(temp_ptr);
        }
        num_commands = 0;
    }
    free(buff);
    return 0;
}

char** extract_commands(char* line, int* commands_num){
    size_t count = 0;
    char* runner = line;

    while((*runner) != '\0'){
        if ((*runner) == '&') count++;
        runner++;
    }

    if(commands_num){
        *commands_num = count + 1; // store the number of commands
    }

    int size = sizeof(char*) * (count + 2); // n ambersands -> n + 1 commands + null terminated array
    char** res = malloc(size);
    res[count + 1] = NULL;

    int i = 0;
    skip_spaces(&line);

    res[i++] = line;

    while((*line) != '\0'){
        if ((*line) == '&'){
            *line = '\0';
            line++;
            skip_spaces(&line);
            res[i++] = line;
            continue;
        }
        line++;
    }
    return res;
}

void execute_command(char* cmd, int is_single) {

    char *outputfile = NULL;
    char* args[MAX_ARGS];
    char *redir = strchr(cmd, '>');

    if (redir != NULL) {
        *redir = '\0';
        redir++;
        skip_spaces(&redir);
        outputfile = redir;
    }

    char* token = strtok(cmd, " ");

    if(token == NULL)
        return;

    if (strcmp(token, "exit") == 0)
        exit(0);
    
    else if(strcmp(token, "cd") == 0){
        token = strtok(NULL, " ");
        if (token == NULL){
            chdir("/");
        }
        else if (chdir(token) != 0){
            perror("cd");
        }
        getcwd(cwd, CWD_MAX);
    }
    else {
        read_args(&token, args);
        if (fork() == 0) {
            if (outputfile){
                int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else {
            if (is_single)
                wait(NULL);
            else
                return;
        }
    }
}

void read_args(char** token, char** args){
    int i = 0;
    while((*token) != NULL && i < MAX_ARGS - 1){
        args[i++] = (*token);
        (*token) = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void skip_spaces(char** str){
    if (!str || !*str) return;
    while (**str && isspace(**str)) (*str)++;
}