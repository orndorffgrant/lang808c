build/lang808c: main.c
	mkdir -p build
	gcc main.c -o build/lang808c

runtest: build/lang808c
	./build/lang808c ./main.l8
