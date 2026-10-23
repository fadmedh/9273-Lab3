
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command::_currentSimpleCommand = new SimpleCommand();

    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].type == TOKEN_EOF) {
            break;
        }
        else if (tokens[i].type == TOKEN_COMMAND || tokens[i].type == TOKEN_ARGUMENT) {
            Command::_currentSimpleCommand->insertArgument(strdup(tokens[i].value.c_str()));
        }
        else if (tokens[i].type == TOKEN_PIPE) {
            Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
            Command::_currentSimpleCommand = new SimpleCommand();
        }
        else if (tokens[i].type == TOKEN_REDIRECT) {
            if (i + 1 < tokens.size()) {
                Command::_currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
                i++;
            }
        }
        else if (tokens[i].type == TOKEN_APPEND) {
            if (i + 1 < tokens.size()) {
                Command::_currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
                Command::_currentCommand._append = 1;
                i++;
            }
        }
        else if (tokens[i].type == TOKEN_INPUT) {
            if (i + 1 < tokens.size()) {
                Command::_currentCommand._inputFile = strdup(tokens[i + 1].value.c_str());
                i++;
            }
        }
        else if (tokens[i].type == TOKEN_BACKGROUND) {
            Command::_currentCommand._background = 1;
        }
    }

    Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);

    
    Command::_currentCommand.execute();
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    if (_numberOfSimpleCommands == 0) {
        prompt();
        return;
    }

    if (strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0) {
        printf("Good bye!!\n");
        exit(0);
    }

    int default_in = dup(0);
    int default_out = dup(1);
    int default_err = dup(2);

    int fin = default_in;
    int fout = default_out;
    int ferr = default_err;

    if (_inputFile) {
        fin = open(_inputFile, O_RDONLY);
        if (fin < 0) {
            perror("Error opening input file");
            prompt();
            return;
        }
    }

    if (_outFile) {
        if (_append) {
            fout = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
        } else {
            fout = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }
        if (fout < 0) {
            perror("Error opening output file");
            prompt();
            return;
        }
    }

    if (_errFile) {
        if (_append) {
            ferr = open(_errFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
        } else {
            ferr = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }
        if (ferr < 0) {
            perror("Error opening error file");
            prompt();
            return;
        }
        dup2(ferr, 2);
        close(ferr);
    }

    dup2(fin, 0);
    if (_inputFile) close(fin);

    int ret = -1;
    int last_fdin = 0;

    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        if (strcmp(_simpleCommands[i]->_arguments[0], "cd") == 0) {
            char *dir = _simpleCommands[i]->_arguments[1];
            if (dir == NULL) {
                dir = getenv("HOME");
            }
            if (chdir(dir) < 0) {
                perror("cd failed");
            }
            continue;
        }

        int fd[2];
        if (i < _numberOfSimpleCommands - 1) {
            pipe(fd);
            fout = fd[1];
            last_fdin = fd[0];
        } else {
            if (_outFile) {
            } else {
                fout = default_out;
            }
        }

        dup2(fout, 1);
        if (fout != default_out && fout != fd[1]) {
            close(fout);
        }
        if (i < _numberOfSimpleCommands - 1) {
            close(fd[1]);
        }

        ret = fork();
        if (ret == 0) {
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror("execvp failed");
            _exit(1);
        } else if (ret < 0) {
            perror("fork failed");
            return;
        }

        if (i < _numberOfSimpleCommands - 1) {
            dup2(last_fdin, 0);
            close(last_fdin);
        }
    }

    dup2(default_in, 0);
    dup2(default_out, 1);
    dup2(default_err, 2);

    close(default_in);
    close(default_out);
    close(default_err);

    if (!_background) {
        waitpid(ret, NULL, 0);  
    }

    clear();
    prompt();
}

void Command::prompt()
{
    printf("myshell>");
    fflush(stdout);
    std::string input;
    while (true)
    {
        std::getline(std::cin, input); 
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;
void sigchld_handler(int sig) {
    int pid;
    int status;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        FILE *logFile = fopen("child_log.txt", "a");
        if (logFile) {
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            time_str[strlen(time_str) - 1] = '\0';
            fprintf(logFile, "Child process %d terminated at %s\n", pid, time_str);
            fclose(logFile);
        }
    }
}
void sigint_handler(int sig) {
    printf("\nmyshell>");
    fflush(stdout);
}
int main()
{
    
   signal(SIGINT, sigint_handler);

    
    signal(SIGCHLD, sigchld_handler);

    Command::_currentCommand.prompt();
    return 0;
}
