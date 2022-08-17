CC=clang++
DEPS=include/$(wildcard *.h)

CFLAGS=@compile_flags.txt

OBJ=main.o utils.o cell.o grid.o lodepng.o entity_player.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main.exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(wildcard *.o)
	rm -f main.exe
