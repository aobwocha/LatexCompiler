all:
	gcc -std=c99 -Wall -Wextra -O2 -c main.c -o main.o
	gcc -std=c99 -Wall -Wextra -O2 -c lexer.c -o lexer.o
	gcc -std=c99 -Wall -Wextra -O2 -c parser.c -o parser.o
	gcc main.o lexer.o parser.o -o latex_compiler

clean:
	rm -f main.o lexer.o parser.o latex_compiler