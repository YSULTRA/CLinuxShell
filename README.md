## CLinuxShell README

### Overview
CLinuxShell is a custom Linux shell implementation written in C, offering a sophisticated command-line environment for executing commands and managing processes. This project is designed for those who need a deeper understanding of process handling and command execution in Unix-like systems.

### Features
- **Command Execution**: Allows execution of both foreground and background processes, supporting pipeline commands.
- **Job Management**: Handles jobs, allowing users to list all background processes and manage them effectively.
- **History Tracking**: Maintains a detailed history of all commands executed, including command details, execution start time, duration, and process IDs.
- **Signal Handling**: Gracefully handles Ctrl+C interrupts, displaying command execution history before exiting.

### Usage
Compile the source with `gcc` and run the executable. Use standard shell commands to interact with the system.

### Compiling and Running
```bash
gcc -o CLinuxShell CLinuxShell.c
./CLinuxShell
```

### Key C Features Used
- **Process Control**: Utilizes `fork()`, `execvp()`, and `waitpid()` for managing child processes.
- **Signal Handling**: Implements custom signal handlers using `signal()` for SIGINT to manage user interruptions.
- **Memory Management**: Employs dynamic memory allocation and deallocation with `malloc()` and `free()` to handle multiple command executions and history tracking.
- **String Manipulation**: Uses functions like `strtok()`, `strncpy()`, and others to parse and handle user inputs and commands effectively.
- **File Descriptor Management**: Implements `pipe()`, `dup2()` for redirecting standard input/output across piped commands.

### Conclusion
CLinuxShell is an educational tool that showcases system-level programming capabilities in C, focusing on process, memory, and job management in a shell environment.

---
