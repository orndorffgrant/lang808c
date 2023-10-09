build/lang808c: main.c common.c common.h lexer.c lexer.h symbols.c symbols.h parser.c parser.h ir.c ir.h
	mkdir -p build 
	gcc main.c common.c lexer.c symbols.c parser.c ir.c -o build/lang808c

runtest: build/lang808c
	./build/lang808c ./examples/main.l8
