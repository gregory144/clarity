CC=clang
CFLAGS=-g `llvm-config --cflags`
LD=clang++
LDFLAGS=`llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

tool: tool.o
	$(LD) tool.o $(LDFLAGS) -o tool -rdynamic

tool.o: tool.c
	$(CC) $(CFLAGS) -c tool.c

clean:
	-rm -rf tool.o tool
.PHONY: clean
