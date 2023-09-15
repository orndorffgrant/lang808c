build/lang808c: main.c common.h lexer.c lexer.h symbols.c symbols.h
	mkdir -p build
	gcc main.c lexer.c symbols.c -o build/lang808c

runtest: build/lang808c
	./build/lang808c ./main.l8
