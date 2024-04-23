#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>

#define MAX_LENGTH 100
#define BOLD "\x1b[1m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define COLOR_RESET "\x1b[0m"
#define RED "\x1b[31m"

// global variables
char input[MAX_LENGTH];
char *line[MAX_LENGTH];
FILE* open_file;
int flag=0;

// functions prototypes
void register_child_signal(void (*fn)(int));
void setup_environment();
void on_child_exit(int signal);
void reap_child_zombie();
void shell();
int check_command();
void execute_shell_builtin();
void execute_command();
void parse_input();
void write_to_log();

// main function
int main() {
	printf("\n");
    printf(BOLD CYAN "========================Welcome to the shell!========================\n" COLOR_RESET);
    register_child_signal(on_child_exit);
    setup_environment();
    shell();
    return 0;
}

// register a signal handler for child processes
void register_child_signal(void (*fn)(int)) {
    signal(SIGCHLD, fn);
}

// set up the initial environment for the shell
void setup_environment() {
    char arr[100];
    chdir(getcwd(arr, sizeof(arr)));
}

//when a child process exits
void on_child_exit(int signal) {
    reap_child_zombie();
    write_to_log();
}
// write to log.txt "Child terminated!"
void write_to_log(){
    open_file = fopen("/home/mennatallah/simpleLinuxShell/log.txt", "a");
    fprintf(open_file, "Child terminated!\n");
    fclose(open_file);
}

// reap zombies
void reap_child_zombie() {
    int status;
    while ((waitpid(-1, &status, WNOHANG)) > 0);
}

// shell
void shell() {
    int not_exit_command; // a flag to check if the user entered "exit", "0" if exit "1" otherwise
    do {
    	int i=0; // input pointer
        printf(MAGENTA"SimpleLinuxShell>> "COLOR_RESET);
        scanf("%[^\n]%*c", input);
        parse_input();
        //check if the user entered the command "exit" if true it will return "0"
        not_exit_command = strcmp(line[0], "exit");
        
		// if the user entered "exit" command we will ignore the rest of the code and the loop will terminate
        if (!not_exit_command){ 
            continue;
        }

        switch(check_command()) {
            case 1:
                execute_shell_builtin();
                break;
            case 0:
                execute_command();
                break;
            default:
                break;
        i++;
        }
    } while (not_exit_command);
    //exit if user typed "exit"
    printf(BOLD CYAN "===============================Byeee!:)===============================\n\n" COLOR_RESET);
    exit(0);
}

//check if a command is a builtin command
int check_command() {
	// shell builtin commands
    char *shell_builtin[] = {"cd", "pwd", "export", "echo"};
    for (int i = 0; i < 4; i++) {
    	// compare the input with the builtin commands
        if (!strcmp(line[0], shell_builtin[i])) {
            return 1;
        }
    }
    return 0;
}

// execute shell builtin commands
void execute_shell_builtin() {
	// check if the command entered by the user is cd
    if (!strcmp(line[0], "cd")) {
    	// check if there is no argument after the cd command
        if (line[1] == NULL) {
            chdir("/home");
            return;
        }
        chdir(line[1]);
        
    // check if the command entered by the user is pwd
    } else if (!strcmp(line[0], "pwd")) {
        char arr[100]; //to store the path of the current working directory
        printf("%s\n", getcwd(arr, sizeof(arr)));
    // check if the command entered by the user is echo
	}else if (!strcmp(line[0], "echo")) {

            char *line1; //pointer to the first argument after "echo" command
            char *line2; //pointer to the second argument after "echo" command
            line1 = line[1];
            
            
            if(line[2]!=NULL) {
                line2 = line[2];
                line2[strlen(line[2])-1]='\0';
                line1++;
                
                // if the first character is "$" print the environment variable
                if(line1[0]=='$'||line2[0]=='$'){
                   if(line1[0]=='$'){
                       line1++;
                       printf("%s ", getenv(line1));
                       
                   } else{
                       printf("%s ",line1);
                   }
                   
                   if(line2[0]=='$'){
                       line2++;
                       printf("%s\n",getenv(line2));
                       
                   }else{
                       printf("%s\n",line2);
                   }
                   
                }else{
                    printf("%s ",line1);
                    printf("%s\n",line2);
                }
            }else{
                line1[strlen(line[1])-1]='\0';
                line1++;

                if(line1[0]=='$') {
                    line1++;
                    printf("%s\n", getenv(line1));

                }else {
                	//split line1 into tokens
                    char* tokens[1000];
                    tokens[0]=strtok(line1,"$");
                    int i = 0;
                    while (tokens[i] != NULL) {
                        i++;
                        tokens[i] = strtok(NULL, "=");

                    }
                    printf("%s",line1);
                    if(tokens[1]!=NULL) {
                        printf("%s\n", getenv(tokens[1]));
                    }else{
                        printf("\n");
                    }
                }
            }

    // check if the command entered by the user is export   
    } else if (!strcmp(line[0], "export")) {
            char *line2;
            line2 = line[2];
            if(line2[0]=='\"'){
				
				//remove " at the end of the string
                line2[strlen(line[2])-1]='\0';
                
                //remove " at the beginning of the string
                line2++;
                setenv(line[1],line2,1);
            }else{
            	//if there are no double quotations
                setenv(line[1],line[2],1);
            }
    } else {
        printf(BOLD RED "Error: Command not found.\n" COLOR_RESET);
    }
}


void execute_command() {
	//create a child process using the fork() system call
    int child_id = fork();

	//check if it's a child process
    if (child_id == 0) {
        int execvp_result; //store the result of execvp
        char *command;//store the command
        if(line[1]!=NULL) {

            command = line[1];
        }else {
            command = line[0];
        }
            char *arr[1000];//to store the command and its arguments


            if (command[0] == '$') {
                command++;//ignore $
                char *env_value = getenv(command);//store the value of the variable

                if(line[1]!=NULL) {
                    arr[0] = line[0];
                    arr[1] = strtok(env_value, " ");

                    int i = 1;
                    while (arr[i] != NULL) {
                        i++;
                        arr[i] = strtok(NULL, " ");

                    }
                     execvp_result= execvp(line[0], arr);
                }else{
                    arr[0] = env_value;
                    execvp_result = execvp(arr[0], arr);
                }

            }else {
                execvp_result = execvp(line[0], line);
            }

        if(execvp_result != 0){
            printf(BOLD RED "Error: Command not found.\n" COLOR_RESET);
            exit(execvp_result);
        }

        exit(0);
    } else if (child_id == -1) {
        printf(BOLD RED "Error: Fork failed!" COLOR_RESET);
        exit(1);
    }else{
    	if (flag == 1){//background process
    		flag=0;
    	}else{
    		waitpid(child_id, NULL, 0);
    	}
    }
}




// parse user input into tokens
void parse_input() {
	if( input[strlen(input)-1] == '&' ){
        input[strlen(input)-1]='\0';
        flag=1;
    }

    line[0] = strtok(input, " ");
    if(strcmp(line[0],"export")==0){
        int i = 0;
            while (line[i] != NULL) {
                i++;
                line[i] = strtok(NULL, "=");

            }
        }else {
            int i = 0;
            while (line[i] != NULL) {
                i++;
                line[i] = strtok(NULL, " ");

            }
        }


}



