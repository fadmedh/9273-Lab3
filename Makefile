# Target to compile the shell
myshell: command.cc tokenizer.cc
	g++ -O3 command.cc tokenizer.cc -o myshell

# Target to clean up compiled files
clean:
	rm -f myshell *.o