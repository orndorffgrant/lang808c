build/lang808c: main.c common.c common.h lexer.c lexer.h symbols.c symbols.h parser.c parser.h ir.c ir.h armv6m.c armv6m.h linker.c linker.h
	mkdir -p build 
	gcc main.c common.c lexer.c symbols.c parser.c ir.c armv6m.c linker.c -o build/lang808c

out.hex: build/lang808c examples/test.l8
	./build/lang808c ./examples/test.l8 > out.hex

program: out.hex
	openocd -f /usr/share/openocd/scripts/board/atmel_samd21_xplained_pro.cfg -c "program out.hex verify reset exit"