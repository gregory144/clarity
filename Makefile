CC=clang
CFLAGS=-g `llvm-config --cflags`
LD=clang++
LDFLAGS=`llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

tool: tool.o ast.o parser.o codegen.o symbols.o
	$(LD) tool.o ast.o parser.o codegen.o symbols.o $(LDFLAGS) -o tool -rdynamic

tool.o: tool.c
	$(CC) $(CFLAGS) -c tool.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

codegen.o: codegen.c codegen.h
	$(CC) $(CFLAGS) -c codegen.c

symbols.o: symbols.c symbols.h
	$(CC) $(CFLAGS) -c symbols.c

clean:
	-rm -rf *.o tool
.PHONY: clean
