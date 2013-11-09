CC=clang
CFLAGS=-g `llvm-config --cflags` -O0
LD=clang++
LDFLAGS=`llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

tool: tool.o ast.o parse.o codegen.o symbol.o enums.o list.o graphgen.o context.o type.o
	$(LD) tool.o ast.o parse.o codegen.o symbol.o enums.o list.o graphgen.o context.o type.o $(LDFLAGS) -o tool -rdynamic

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

context.o: context.c context.h
	$(CC) $(CFLAGS) -c context.c

type.o: type.c type.h
	$(CC) $(CFLAGS) -c type.c

graph: graph.dot
	dot -Tsvg graph.dot > graph.svg

clean:
	-rm -rf *.o tool graph.svg
.PHONY: clean
