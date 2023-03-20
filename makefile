CC=clang++
DEPS=$(wildcard include/*.h)

CFLAGS=@compile_flags.txt

OBJ=main.o utils.o cell.o grid.o quarry.o sprite.o shape.o rigidbody.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -framework openGL

SFML_OBJ=vendor/imgui/imguilib.a

main.exe: $(OBJ) $(SFML_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -framework openGL

clean:
	rm -f imgui.ini
	rm -f $(wildcard *.o)
	rm -f main.exe

