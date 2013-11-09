PROGRAM = tool
C_FILES := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(C_FILES))

CC=clang
CFLAGS=-g `llvm-config --cflags` -O0
LD=clang++
LDFLAGS=`llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

$(PROGRAM): $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@ -rdynamic

# These are the pattern matching rules. In addition to the automatic
# variables used here, the variable $* that matches whatever % stands for
# can be useful in special cases.
%.o: %.c
		$(CC) $(CFLAGS) -c $< -o $@

%: %.c
		$(CC) $(CFLAGS) -o $@ $<

graph: graph.dot
	dot -Tsvg graph.dot > graph.svg

clean:
	-rm -rf *.o tool graph.svg
.PHONY: clean

