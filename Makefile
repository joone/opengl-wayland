LIBS = -lGLESv2 -lEGL -lm -lX11  -lcairo -lwayland-client -lwayland-server -lwayland-cursor -lwayland-egl
CFLAGS = -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libdrm -I/usr/include/libpng12  -I/usr/include

all: triangle_animation triangle \

triangle_animation : ./2.triangle_animation/main.o  ./common/common.o ./common/window.o
	gcc ./2.triangle_animation/main.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}
triangle : ./1.triangle/main.o  ./common/common.o ./common/window.o
	gcc ./1.triangle/main.c ./common/common.c ./common/window.c ${CFLAGS} -o $@ ${LIBS}



clean:
	rm -f *.o *~ 
