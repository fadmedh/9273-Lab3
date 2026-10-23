# MyShell - Custom Linux Shell in C++

A lightweight custom Unix shell implementation built in C++ as part of Operating Systems coursework.

## Features Implemented
* **Command Execution:** Supports running standard system commands with arguments (both foreground and background using `&`).
* **I/O Redirection:** Handles standard input (`<`), output (`>`), and append output (`>>`), along with standard error redirection and appending (`2>` / `>>&`).
* **Pipes (`|`):** Connects the output of one command as the input to another.
* **Signal Handling:** 
  * `SIGINT` (Ctrl+C) handling to prevent the shell from terminating accidentally.
  * `SIGCHLD` handling to automatically clean up background zombie processes and log their termination details.
* **Logging System:** Automatically tracks and appends completed background child process PIDs and timestamps into `child_log.txt`.

## How to Compile & Run

1. Open a terminal in the project directory.
2. Compile the project using Makefile:
   ```bash
   make
Run your custom shell:
./myshell
Exit the shell:
myshell> exit