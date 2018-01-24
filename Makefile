LIBS = -lGLESv2 -lEGL -lm -lX11  -lcairo -lwayland-client -lwayland-server -lwayland-cursor -lwayland-egl
CFLAGS = -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libdrm -I/usr/include/libpng12  -I/usr/include

all: triangle_animation triangle triangle_simple simple_texture rotate_texture triangle_color \

triangle_animation : ./2.triangle_animation/main.o  ./common/common.o ./common/window.o
	gcc ./2.triangle_animation/main.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}
triangle : ./1.triangle/main.o  ./common/common.o ./common/window.o
	gcc ./1.triangle/main.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}

triangle_simple : ./3.triangle_simple/triangle.o  ./common/common.o ./common/window.o
	gcc ./3.triangle_simple/triangle.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}

simple_texture : ./4.simple_texture/main.o  ./common/common.o ./common/window.o
	gcc ./4.simple_texture/main.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}

rotate_texture : ./5.rotate_texture/main.o  ./common/common.o ./common/window.o
	gcc ./5.rotate_texture/main.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}

triangle_color : ./6.triangle_color/main.o  ./common/common.o ./common/window.o
	gcc ./6.triangle_color/main.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}

clean:
	rm -f 1.triangle/*.o *~ 
	rm -f triangle
	rm -f 2.triangle_animation/*.o *~ 
	rm -f triangle_animation
	rm -f 3.triangle_simple/*.o *~ 
	rm -f triangle_simple
	rm -f 4.simple_texture/*.o *~ 
	rm -f simple_texture
	rm -f 5.rotate_texture/*.o *~ 
	rm -f rotate_texture
	rm -f 6.triangle_color/*.o *~ 
	rm -f triangle_color
