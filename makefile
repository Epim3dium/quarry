CC=clang++
DEPS=include/$(wildcard *.h)

CFLAGS=@compile_flags.txt

OBJ=main.o utils.o cell.o grid.o lodepng.o entity_player.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

SFML_OBJ=imgui/imgui-SFML.o imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o imgui/imgui_tables.o imgui/imgui_widgets.o


main.exe: $(OBJ) $(SFML_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(wildcard *.o)
	rm -f main.exe
