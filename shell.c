#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<fnctl.h>


void handle_sigint(int sig){//signal handle
    char cwd[1024];
    if(getcwd(cwd,sizeof(cwd)) != NULL)
    printf("Caught single %d. Type 'exit' to quit.\n%s my_shell>",sig,cwd);
    else
    printf("Caught single %d. Type 'exit' to quit.\nmy_shell>",sig);
    fflush(stdout);
}

int main(){
    char input[1024];
    char* args[64];
    char cwd[1024];

    signal(SIGINT,handle_sigint);

    while(1){
        if(getcwd(cwd,sizeof(cwd)) != NULL){
            printf("%s my_shell-> ",cwd);
        }
        else
        printf("my_shell-> ");
        if(fgets(input,sizeof(input),stdin)==NULL) break;
        input[strcspn(input,"\n")] = 0;
        if(strlen(input) == 0) continue;
        int i =0;
        char* token = strtok(input," "); //create a pointer that points to location of cut 
        while(token != NULL){
            args[i] = token; // assign it to the args to store
            i++;
            token = strtok(NULL," "); // move to next cut
        }
        args[i] = NULL; // tells exec() to stop
        if(strcmp(args[0],"exit") == 0){
            printf("GOOD-BYE!\n");
            break;
        }

        if(strcmp(args[0],"cd")==0){
            if(args[1] == NULL){
                fprintf(stderr, "cd:expected argument\n");
            }
            else{
                if(chdir(args[1]) !=0){
                    perror("cd failed\n");
                }
            }
            continue;
        }
        pid_t pid = fork();
        if(pid<0){
            perror("fork failed ! \n");
        }
        else if (pid == 0){
            //fork successful child is now running
            if(execvp(args[0],args) == -1){ // execution failed
                printf("%s: Command Not Found ! \n",args[0]);
            }
            exit(1);
        } 
        else
        wait(NULL);
    }
    return 0;
}