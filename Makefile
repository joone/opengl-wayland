LIBS = -lGLESv2 -lEGL -lm -lX11  -lcairo -lwayland-client -lwayland-server -lwayland-cursor -lwayland-egl
CFLAGS =-g -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libdrm -I/usr/include/libpng12  -I/usr/include

all: triangle triangle_animation triangle_simple simple_texture rotate_texture triangle_color mvp_triangle cube \

triangle : 
	g++ ./1.triangle/main.cc ./common/wayland_platform.cc ./common/gl.cc ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

triangle_animation : 
	g++ ./2.triangle_animation/main.cc ./common/wayland_platform.cc ./common/gl.cc ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

triangle_simple : 
	g++ ./3.triangle_simple/triangle.cc ./common/wayland_platform.cc ./common/gl.cc ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

simple_texture : 
	g++ ./4.simple_texture/main.cc ./common/wayland_platform.cc ./common/gl.cc ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

rotate_texture : 
	g++ ./5.rotate_texture/main.cc ./common/wayland_platform.cc ./common/gl.cc ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

triangle_color : 
	g++ ./6.triangle_color/main.cc ./common/wayland_platform.cc ./common/gl.cc ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

mvp_triangle :
	g++ ./7.mvp_triangle/main.cc ./common/wayland_platform.cc ./common/gl.cc ./common/matrix.cpp ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

cube : 
	g++ ./8.cube/main.cc ./common/wayland_platform.cc ./common/gl.cc ./common/matrix.cpp ./common/display.cc ./common/window.cc ${CFLAGS} -o $@ ${LIBS}

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
	rm -f 7.mvp_triangle/*.o *~ 
	rm -f mvp_triangle
	rm -f 8.cube/*.o *~ 
	rm -f cube
