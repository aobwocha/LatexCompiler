all:
	mkdir -p output
	gcc -std=c99 -Wall -Werror -c src/main.c -o output/main.o
	gcc -std=c99 -Wall -Werror -c src/lexer.c -o output/lexer.o
	gcc -std=c99 -Wall -Werror -c src/parser.c -o output/parser.o
	gcc output/main.o output/lexer.o output/parser.o -o output/latex_compiler

clean:
	rm -rf output lexer_output.txt parser_output.txt