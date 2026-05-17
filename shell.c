#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<fcntl.h>

void handle_sigint(int sig) { // signal handle
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) != NULL)
        printf("\nCaught signal %d. Type 'exit' to quit.\n%s my_shell> ", sig, cwd);
    else
        printf("\nCaught signal %d. Type 'exit' to quit.\nmy_shell> ", sig);
    fflush(stdout);
}

int main() {
    char input[1024];
    char* args[64];
    char cwd[1024];
    char* args_pipe[64];

    signal(SIGINT, handle_sigint);

    while(1) {
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s my_shell-> ", cwd);
        }
        else {
            printf("my_shell-> ");
        }
        
        if(fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;
        if(strlen(input) == 0) continue;
        
        int i = 0;
        char* token = strtok(input, " "); 
        while(token != NULL) {
            args[i] = token; 
            i++;
            token = strtok(NULL, " "); 
        }
        args[i] = NULL; 

        // exit command
        if(strcmp(args[0], "exit") == 0) {
            printf("GOOD-BYE!\n");
            break;
        }

        // managing built in command cd
        if(strcmp(args[0], "cd") == 0) {
            if(args[1] == NULL) {
                fprintf(stderr, "cd: expected argument\n");
            }
            else {
                if(chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        }

        // manage pipes and redirection
        int has_pipe = 0;
        int pipe_index = -1;
        for(int j = 0; args[j] != NULL; j++) {
            if(strcmp(args[j], "|") == 0) {
                has_pipe = 1;
                pipe_index = j;
                args[j] = NULL;
                break;
            }
        }

        if(has_pipe) {
            int k = 0;
            for(int j = pipe_index + 1; args[j] != NULL; j++) {
                args_pipe[k++] = args[j];
            }
            args_pipe[k] = NULL;
        }

        if(has_pipe) {
            int pipefd[2];
            if(pipe(pipefd) == -1) {
                perror("pipe failed !\n");
                continue;
            }

            pid_t pid1 = fork();
            if(pid1 == 0) {
                signal(SIGINT, SIG_DFL);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                if(execvp(args[0], args) == -1) {
                    printf("%s: Command Not Found ! \n", args[0]);
                }
                exit(1);
            }
            
            pid_t pid2 = fork();
            if(pid2 == 0) {
                signal(SIGINT, SIG_DFL);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                if(execvp(args_pipe[0], args_pipe) == -1) {
                    printf("%s: Command Not Found ! \n", args_pipe[0]);
                }
                exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
        else { 
            // managing files and redirections
            char* outfile = NULL;
            char* infile = NULL;
            for(int j = 0; j < i; j++) {
                if(strcmp(args[j], ">") == 0) {
                    outfile = args[j+1];
                    args[j] = NULL;
                    break;
                }
                else if(strcmp(args[j], "<") == 0) {
                    infile = args[j+1];
                    args[j] = NULL;
                    break;
                }
            }
            
            pid_t pid = fork();
            if(pid < 0) {
                perror("fork failed ! \n");
            }
            else if (pid == 0) {
                signal(SIGINT, SIG_DFL);
                
                if(outfile != NULL) {
                    int fd = open(outfile, O_CREAT | O_TRUNC | O_WRONLY, 0644);
                    if(fd < 0) {
                        perror("Failed to open outfile !\n");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                
                if(infile != NULL) {
                    int fd = open(infile, O_RDONLY);
                    if(fd < 0) {
                        perror("Failed to open infile !\n");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                if(execvp(args[0], args) == -1) { 
                    printf("%s: Command Not Found ! \n", args[0]);
                }
                exit(1);
            } 
            else {
                wait(NULL);
            }
        }
    } // Closes the while(1) loop
    return 0;
}