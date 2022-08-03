CC=clang++
DEPS=include/$(wildcard *.h)

CFLAGS=@compile_flags.txt

OBJ=build/main.o build/utils.o build/cell.o build/grid.o build/quad_tree.o

build/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main.exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
