TEST_OBJ=obj/test.o obj/sim.o obj/misc.o
WND_OBJ=obj/wnd.o obj/draw.o obj/sim.o obj/misc.o obj/gl.o
VID_OBJ=obj/vid.o obj/draw.o obj/sim.o obj/misc.o obj/gl.o

bin/test: bin obj $(TEST_OBJ)
	gcc $(TEST_OBJ) -lm -o $@ 

bin/vid: bin obj $(VID_OBJ) 
	gcc $(VID_OBJ) -o $@ -lSDL2main -lSDL2 -lm -lswscale \
		-lavcodec -lavformat -lavutil -lx264 

bin/wnd: bin obj $(WND_OBJ)
	gcc $(WND_OBJ) -lm -lSDL2main -lSDL2 -o $@ 

obj/vid.o: src/vid.c src/sim.h src/draw.h src/misc.h
	gcc $< -o $@ -Ilib/cglm/include -Ilib/glad/include -c

obj/test.o: src/test.c src/sim.h
	gcc $< -o $@ -Ilib/cglm/include -c

obj/sim.o: src/sim.c src/sim.h src/misc.h
	gcc $< -o $@ -Ilib/cglm/include -c -O3 -fno-strict-aliasing -Wall -Wextra


obj/misc.o: src/misc.c src/misc.h
	gcc $< -o $@ -c

obj/draw.o: src/draw.c src/draw.h src/misc.h src/sim.h
	gcc $< -o $@ -Ilib/cglm/include -Ilib/glad/include -c

obj/wnd.o: src/wnd.c src/draw.h src/sim.h
	gcc $< -o $@ -Ilib/cglm/include -Ilib/glad/include -c

bin:
	mkdir bin

obj:
	mkdir obj

obj/gl.o: lib/glad/src/gl.c 
	gcc $< -o $@ -Ilib/glad/include -c

clean:
	rm obj bin -rf
