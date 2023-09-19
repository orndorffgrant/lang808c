build/lang808c: main.c common.c common.h lexer.c lexer.h
	mkdir -p build 
	gcc main.c common.c lexer.c -o build/lang808c

runtest: build/lang808c
	./build/lang808c ./main.l8
