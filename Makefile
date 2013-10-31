CC=clang
CFLAGS=-g `llvm-config --cflags` -O0
LD=clang++
LDFLAGS=`llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

tool: tool.o ast.o parse.o codegen.o symbol.o enums.o list.o graphgen.o
	$(LD) tool.o ast.o parse.o codegen.o symbol.o enums.o list.o graphgen.o $(LDFLAGS) -o tool -rdynamic

tool.o: tool.c
	$(CC) $(CFLAGS) -c tool.c

parse.o: parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

codegen.o: codegen.c codegen.h
	$(CC) $(CFLAGS) -c codegen.c

symbol.o: symbol.c symbol.h
	$(CC) $(CFLAGS) -c symbol.c

enums.o: enums.c enums.h
	$(CC) $(CFLAGS) -c enums.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

graphgen.o: graphgen.c graphgen.h
	$(CC) $(CFLAGS) -c graphgen.c

graph: graph.dot
	dot -Tsvg graph.dot > graph.svg

clean:
	-rm -rf *.o tool
.PHONY: clean
