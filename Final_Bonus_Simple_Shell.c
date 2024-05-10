/*With Bonus Complete Code*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMANDS 10

//This structure is a Linked list, storing the command given by the user in each node:
typedef struct Node {
    char element[1024];
    struct Node* next;
} Node;

//This structure is a Linked List, each node is storing data of individual commands:
typedef struct DetailedNode {
    char element[1024];
    pid_t processID;
    struct timeval startTime;
    double duration;
    struct DetailedNode* next;
} DetailedNode;

//This structure is Linked List, each node is storing data of the executing command:
typedef struct Job {
    pid_t processID;
    char command[1024];
    int running; // 1 if running, 0 if done
    struct Job* next;
} Job;

// Defining Three Linked list to store data of the commands given by the user:
Job* jobsHead = NULL;
DetailedNode* detailedHistoryHead = NULL; // Declare as a global variable
Node* commandHistoryHead = NULL;
// Declaring the global varible:
int ctrl_c_pressed = 0;

//This Function updates the Linked List "Node" with a new node containing the new command given:
void appendToCommandHistory(Node** headRef, const char* newData) {
    // Allocate memory for the new node
    Node* newNode = (Node*)malloc(sizeof(Node));
    //Returns error if the allocation of the memory for new node is failed.
    if (newNode == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }

    // Set the data of the new node
    // Copy a string from 'newData' into the 'element' field of a Node structure named "newNode".
    strncpy(newNode->element, newData, sizeof(newNode->element));
    newNode->element[sizeof(newNode->element) - 1] = '\0';  // Ensure null-terminated string is at the end.
    newNode->next = NULL;  // Setting the address in the next field as 'NULL' as it is the last node.

    // If the list is empty,then making the new node as the head.
    if (*headRef == NULL) {
        *headRef = newNode;
    } else {
        // Traverse to the last node and append the new node
        Node* current = *headRef;
        while (current->next != NULL) {
            current = current->next;
        }
        // Traversing to the last node and appending the new node address in the last node next field.
        current->next = newNode;
    }
}

//This Function updates the Linked List 'DetailedNode" with a new node containing the new command details:
void appendToDetailedHistory(DetailedNode** headRef, const char* newData, pid_t processID, struct timeval startTime, double duration) {
    // Allocate memory for the new node
    DetailedNode* newNode = (DetailedNode*)malloc(sizeof(DetailedNode));
    if (newNode == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }

    // Set the data of the new node
    // Copy a string from 'newData' into the 'element' field of a DetailedNode structure named "newNode".
    strncpy(newNode->element, newData, sizeof(newNode->element));
    newNode->element[sizeof(newNode->element) - 1] = '\0';  // Ensure null-terminated string at the end.
    newNode->processID = processID; // Stores the process ID.
    newNode->startTime = startTime; // Stores the starting time of the execution of the command.
    newNode->duration = duration;   // Stores the duration of execution of the command.
    newNode->next = NULL;  // Setting the address in the next field as 'NULL' as it is the last node.

    // If the list is empty,then making the new node as the head
    if (*headRef == NULL) {
        *headRef = newNode;
    } else {
        // Traversing to the last node and then appending the new node in the linked list.
        DetailedNode* current = *headRef;
        while (current->next != NULL) {
            current = current->next;
        }
        //Storing address of the new node in the 'next' field of last node of the linked list.
        current->next = newNode;
    }
}

//Functions to print history of the command given by the user:
void printCommandHistory(Node* head) {
    Node* current = head;
    int count = 1; // Initialize a counter for index numbers

    while (current != NULL) {
        printf("%d: %s\n", count, current->element); // Print index and command
        current = current->next;
        count++;
    }
}

//This Function transverse through the linked list and prints all the data of previous command given:
void printDetailedHistory(DetailedNode* head) {
    DetailedNode* current = head; //Calling the head node of the linked list.
    int count = 1; // Initializing a counter for index numbers

    //Transvering through the linked list and printing all the information of the previous commands:
    while (current != NULL) {
        printf("%d: Command: %s\n", count, current->element); // Print index and command
        printf("   Process ID: %d\n", current->processID); // Print process ID
        printf("   Start Time: %ld.%06ld\n", current->startTime.tv_sec, current->startTime.tv_usec); // Print start time
        printf("   Duration: %.6f seconds\n", current->duration); // Print duration
        current = current->next; //Switching to the next node
        count++;
    }
}

//This Function manages Memory Allocation, Data Initialization, Linked List Handling of the jobs:
void appendToJobs(Job** headRef, pid_t processID, const char* command) {
    // Allocate memory for the new job
    Job* newJob = (Job*)malloc(sizeof(Job));
    if (newJob == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }

    // Set the data of the new job
    newJob->processID = processID;

    // Copy the command into the new job's command field
    strncpy(newJob->command, command, sizeof(newJob->command));
    
    // Ensure that the command field is null-terminated
    newJob->command[sizeof(newJob->command) - 1] = '\0';

    // Set the job as running initially
    newJob->running = 1;

    // Since this is the last job in the list, set its 'next' pointer to NULL
    newJob->next = NULL;

    // If the list is empty, make the new job the head
    if (*headRef == NULL) {
        *headRef = newJob;
    } else {
        // Traverse the list to find the last job and append the new job
        Job* current = *headRef;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newJob;
    }
}

//This function is responsible for removing finished jobs from a linked list of jobs:
void removeFinishedJobs(Job** headRef) {
    // Initialize pointers to traverse and manipulate the job list
    Job* current = *headRef; // Pointer to the current job
    Job* prev = NULL; // Pointer to the previous job

    // Iterate through the job list
    while (current != NULL) {
        // Variables for storing process status and result of waitpid
        int status;
        pid_t result = waitpid(current->processID, &status, WNOHANG);

        // Check if the process has finished or if there was an error in waitpid
        if (result == -1 || result == current->processID) {
            // Process has finished, remove it from the list

            // Adjust list pointers to remove the current job
            if (prev == NULL) {
                // If the current job is the head, update the head to the next job
                *headRef = current->next;
            } else {
                // If the current job is not the head, bypass it by updating the previous job's 'next' pointer
                prev->next = current->next;
            }

            // Free memory associated with the current job to prevent memory leaks
            free(current);

            // Revisit the previous job (if any) in the next iteration
            current = prev;
        }

        // Move to the next job in the list
        prev = current;
        if (current != NULL) {
            current = current->next;
        }
    }
}

// This function is responsible for printing information about jobs in a linked list:
void printJobs(Job* head) {
    // Initialize a pointer to traverse the job list
    Job* current = head; // Pointer to the current job

    // Initialize a counter to keep track of job numbers
    int count = 1; // Job count starts from 1

    // Iterate through the job list
    while (current != NULL) {
        // Print job information, including job number, status (Running or Done), and the command
        printf("[%d] %s %s\n", count, current->running ? "Running" : "Done", current->command);

        // Move to the next job in the list
        current = current->next;

        // Increment the job count for the next job
        count++;
    }
}

// This function is responsible for releasing the memory associated with a linked list of jobs:
void freeJobs(Job* head) {
    // Initialize a pointer to traverse the job list
    Job* current = head; // Pointer to the current job

    // Iterate through the job list
    while (current != NULL) {
        // Create a temporary pointer to store the current job
        Job* temp = current;

        // Move to the next job in the list
        current = current->next;

        // Free the memory associated with the current job
        free(temp);
    }
}

//This Function is clears all the data in the Linked List "Node":
void freeCommandHistory(Node* head) {
    Node* current = head;
    while (current != NULL) { // Transversing through the linked list until the last node has come.
        Node* temp = current;
        current = current->next; // Switching the current pointer to next node's address.
        free(temp); // Clearing the node.
    }
}

////This Function is clears all the data in the Linked List "DetailedNode":
void freeDetailedHistory(DetailedNode* head) {
    DetailedNode* current = head;
    while (current != NULL) { // Transversing through the linked list until the last node has come.
        DetailedNode* temp = current; 
        current = current->next; // Switching the current pointer to next node's address.
        free(temp); // Clearing the node.
    }
}

// This Function takes a path to a shell script, constructs a command to execute the script:
int execute_shell_script(const char* script_path) {
    // Create a character array to store the shell command to execute the script
    char command[1024];

    // Use snprintf to construct the command by appending the script path
    // This assumes that the script will be executed using '/bin/bash'
    snprintf(command, sizeof(command), "/bin/bash %s", script_path);

    // Use the system function to execute the constructed command
    int result = system(command);

    // Check if the system function returned an error (-1)
    if (result == -1) {
        // Print an error message using perror and return an error code (1)
        perror("Failed to execute shell script");
        return 1;
    }

    // Extract the exit status of the executed shell script using WEXITSTATUS
    return WEXITSTATUS(result);
}

//This Function is for the single command execution with no pipeline:
int execute_single_command(char *command) {
    if (strchr(command, '\\') || strchr(command, '\'') || strchr(command, '\"')) {
        printf("Error: Backslashes and quotes are not allowed.\n");
        return 1;  
    } 
    // Single command execution and there is no pipes.
    // Calling function to Execute and storing the output in result.
    char *args[1024];
    char *parsed_command;
    parsed_command = strtok(command, " ");
    int i = 0;

    while (parsed_command != NULL) {
        args[i++] = parsed_command;
        parsed_command = strtok(NULL, " ");
    }
    args[i] = NULL;
    //Making another Child Process:
    pid_t pid = fork();

    if (pid < 0) {
        // Creation of child process is failed.
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Code for Child process execution.
        if (execvp(args[0], args) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        // Making the Parent process wait until the child process are done from execution.
        wait(NULL);
    }

    return 0;
}

//This Function executes the commands given by the user:
int fork_create_process(char command[]) {
    // Removes the newline character and replaces it with the null terminator.
    command[strcspn(command, "\n")] = '\0';

    struct timeval startTime;
    gettimeofday(&startTime, NULL); //Obtains the current time and stores it in startTime
    
    //Bonus: Shell Scripting Section;
    if (strstr(command, ".sh") != NULL) {
        int script_exit_status = execute_shell_script(command);
        struct timeval endTime;
        gettimeofday(&endTime, NULL);
        double duration = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1e6;
        appendToDetailedHistory(&detailedHistoryHead, command, getpid(), startTime, duration);
        return script_exit_status;
    }

    // Checking for the presence of pipes
    // Array of character pointers:
    char *pipe_commands[1024];
    int pipe_count = 0;
    // Spliting the string given by the user into individual commands and storing it.
    char *parsed_command = strtok(command, "|");

    //Looping until there are no more commands:
    while (parsed_command != NULL) {
        pipe_commands[pipe_count] = parsed_command;
        parsed_command = strtok(NULL, "|");
        pipe_count++;
    }

    pid_t pids[1024];
    double durations[1024];

    //EXECUTION OF SINGLE COMMAND EXECUTION:
    if (pipe_count == 1) {
        // Single command, no pipes
        int is_background = 0;

        // Check if the command ends with "&" for background execution
        if (command[strlen(command) - 1] == '&') {
            is_background = 1;
            command[strlen(command) - 1] = '\0'; // Remove the "&" symbol
        }

        int result = execute_single_command(pipe_commands[0]);
        struct timeval endTime;
        // Obtains the current time and stores it in endTime after execution of the command
        gettimeofday(&endTime,NULL);
        // Calculating and Storing the execution time of the single pipeline command.
        double duration =(endTime.tv_sec-startTime.tv_sec)+(endTime.tv_usec-startTime.tv_usec)/1e6;
        // Updating the Detailed History Linked List.
        appendToDetailedHistory(&detailedHistoryHead, pipe_commands[0], getpid(), startTime, duration);

        return result;
    }
    //EXECUTION OF MULTIPLE COMMAND EXECUTION:
    else if (pipe_count > 1) { // Multiple commands in a pipeline.
        // Calling function to Execute multiple commands and storing the output in result.
        //2D Array named 'pipes' is declared to hold information for pipes as: [read end, write end].
        int pipes[pipe_count - 1][2];

        //Creating Pipe for each of the command:
        for (int i = 0; i < pipe_count - 1; i++) {
            if (pipe(pipes[i]) == -1) { // prints Error if a pipe creation fails:
                perror("Pipe creation failed");
                exit(1);
            }
        }

        //Iterates through the list of commands:
        for (int i = 0; i < pipe_count; i++) {
            //Declaring Array to store the commands.
            char *args[1024];
            char *parsed_command;
            // Store the command string.
            parsed_command = strtok(pipe_commands[i], " ");
            int j = 0;

            // Stores them in the args array as the individual arguments not as the whole command string
            while (parsed_command != NULL) {
                args[j++] = parsed_command;
                parsed_command = strtok(NULL, " ");
            }
            //Sets the last element of the args array to NULL to mark the end of the arguments.
            args[j] = NULL;

            //Creating a Child Process
            pid_t pid = fork();

            //If formation of Child Process Fails then, it reports an error. 
            if (pid < 0) {
                perror("Fork failed");
                exit(1);
            } else if (pid == 0) { // Code Execution for the Child process:
                if (i < pipe_count - 1) {
                    // Redirecting the output to the next pipe:
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                if (i > 0) {
                    // Redirecting the input from the previous pipe
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }

                // Close all pipe ends
                for (int k = 0; k < pipe_count - 1; k++) {
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }

                // Execute the specified command and if execution fails, it prints an error message and exits.
                if (execvp(args[0], args) == -1) {
                    perror("Command execution failed");
                    exit(1);
                }
            } else {
                pids[i] = pid;
            }
        }

        // Close all pipes that ends in the parent process
        for (int i = 0; i < pipe_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // Waiting for all child processes to finish:
        for (int i = 0; i < pipe_count; i++) {
            wait(NULL);
        }


        // Waiting for each process to finish and then calculating the duration of each command
        for (int i = 0; i < pipe_count; i++) {
            int status;
            //waiting for the termination of a specific process specified by 
            // 1) pids[i] (Process ID). 2) &status (exit status of the process)
            waitpid(pids[i], &status, 0);

            struct timeval endTime;
            //Obtains the current time and stores it in endTime after execution of the command
            gettimeofday(&endTime, NULL);
            durations[i] = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1e6;
            // Updating the Detailed History Linked List.
            appendToDetailedHistory(&detailedHistoryHead, pipe_commands[i], pids[i], startTime, durations[i]);
        }
        return 0;
    }

    // IF Pipe count is neither 1 nor greater than 1 then printing "Invalid input"
    printf("Invalid command format: %s\n", command);
    return 1;
}

//This Function executes any command in the background and handle error according to it/
//It also records the start time, adds the background job to the jobs list, 
//and prints job information to notify the user about the background job's PID and command:
void background_execute(char command[]) {
    // Remove the newline character (if present) from the command
    command[strcspn(command, "\n")] = '\0';

    // Create a new process for the background command
    pid_t pid = fork();

    if (pid < 0) {
        // Handle fork failure by printing an error message and exiting
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // // Execute the background command using fork_create_process of child process
        fork_create_process(command);
        exit(0);
    } else {
        // Parent process
        struct timeval startTime;
        // Record the start time for measuring command execution duration
        gettimeofday(&startTime, NULL);

        // Add the background job to the jobs list
        appendToJobs(&jobsHead, pid, command);
        struct timeval endTime;
        gettimeofday(&endTime, NULL);
        double duration = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1e6;
        appendToDetailedHistory(&detailedHistoryHead, command, pid, startTime, duration);

        // Print job information to inform the user
        printf("[%d] %s\n", pid, command);
    }
}

//This Function calls the printing of jobs commands:
void jobs_command() {
    // Print the list of background jobs and their status.
    printf("Jobs:\n");
    printJobs(jobsHead);

    // Remove finished jobs from the list.
    removeFinishedJobs(&jobsHead);
}

//This Function ends the program and prints all the history of the commands given by the user:
void ctrl_c_handler(int signal) {
    // Set the flag to indicate Ctrl+C was pressed
    ctrl_c_pressed = 1;
    printf("\n");
    printDetailedHistory(detailedHistoryHead);
    exit(0);
}

//This Function registers a signal handler for Ctrl+C (SIGINT).
//It represents the core logic of a shell that continuously accepts user input, executes commands 
//in the foreground or background, maintains a command history, and records detailed information about command execution:
void infinite_loop_shell() {

    // Register a signal handler for the Ctrl+C signal (SIGINT)
    signal(SIGINT, ctrl_c_handler);

    // Initialize a character array to store the user's command
    char command[1024];
    // Initialize a variable to store the exit status of the last foreground command
    int last_foreground_status = 0; 

    // Start an infinite loop for the shell
    do {
        // Print the shell prompt
        printf("yash&yogesh~$ ");
        // Read the user's input command from stdin
        fgets(command, sizeof(command), stdin);

        // Remove the newline character from the command (if present)
        command[strcspn(command, "\n")] = '\0';

        // Check if the user entered the "history" command
        if(strcmp(command, "history") == 0) {
            // Print the command history
            printf("Command History:\n");
            printCommandHistory(commandHistoryHead);
        } else if (strcmp(command, "jobs") == 0) {
            // List background jobs
            jobs_command();
        } else {
            // Check if the command should run in the background (ends with '&')
            if (command[strlen(command) - 1] == '&') {
                // Background execution, remove the "&" symbol
                appendToCommandHistory(&commandHistoryHead, command);
                command[strlen(command) - 1] = '\0';
                // Execute the command in the background
                
                background_execute(command);
            } else {
                // Foreground execution, wait for the child process to finish
                appendToCommandHistory(&commandHistoryHead, command);
                // Create a new process (fork)
                pid_t pid = fork();
                if (pid < 0) {
                    perror("Fork failed");
                    exit(1);
                } else if (pid == 0) {
                    // Execute the command in the foreground for the Child Process
                    fork_create_process(command);
                    // Exit the child process
                    exit(0);
                } else {
                    // Parent process Code Execution:
                    struct timeval startTime;
                    // Record the start time for measuring command execution duration
                    gettimeofday(&startTime, NULL);

                    // Wait for the child process to finish and collect its exit status
                    int status;
                    waitpid(pid, &status, 0);
                    last_foreground_status = status; // Store the exit status of the foreground command

                    // Record the end time for measuring command execution duration
                    struct timeval endTime;
                    gettimeofday(&endTime, NULL);

                    // Calculate the duration of command execution
                    double duration = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1e6;
                    // Add detailed information about the command to the detailed history
                    appendToDetailedHistory(&detailedHistoryHead, command, pid, startTime, duration);
                }
            }
        }
    } while (1);

    // After the loop, you can display the exit status of the last foreground command if needed
    printf("[%d] Done: %d\n", getpid(), last_foreground_status);
}

//Main Function:
int main() {
    // Reading "Function Command" from the standard input:
    //Description: 
    // 1) Executes these commands as separate processes.
    // 2) Maintains a command history.
    // 3) It prints command history and can exit the shell by Ctrl+C or exit command.
    // 4) Bonus Functions Funcionalities such as shell scripting etc. 
    //Creating a String to store "User Input"
    infinite_loop_shell();

    //Both Functions clears all the details of the commands stored in the Linked List:
    freeCommandHistory(commandHistoryHead);
    freeDetailedHistory(detailedHistoryHead);
    return 0;
}