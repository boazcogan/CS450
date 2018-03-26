# Makefile for pipe test

all: run_me.x one_pipe.x main.x pipes.x pipe_demo.x

# $@ is make shorthand for the thing on the left side of the colon
#   (pipes.x in this case)
# $^ stands for everything on the right side of the colon (the .o files)
run_me.x: multiple_pipes.c parsetools.o
	gcc -g -o $@ $^

one_pipe.x: one_pipe.c parsetools.o
	gcc -g -o $@ $^

main.x: execute_cmd.c parsetools.o
	gcc -g -o $@ $^

pipes.x: main.o parsetools.o
	gcc -g -o $@ $^

pipe_demo.x: pipe_demo.o
	gcc -g -o $@ $^

# $< is the first item after the colon (main.c here)
main.o: main.c parsetools.h constants.h
	gcc -c -o $@ $<

parsetools.o: parsetools.c constants.h
	gcc -c -o $@ $<

pipe_demo.o: pipe_demo.c
	gcc -c -o $@ $<

clean:
	rm -f *.x *.o *~

push:
	make clean
	git add .
