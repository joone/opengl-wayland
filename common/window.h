#ifndef WINDOW_H_
#define WINDOW_H_

#include <GLES2/gl2.h>

#include "display.h"

static int running = 1;

typedef struct {
   GLfloat   m[4][4];
} ESMatrix;

struct geometry {
  int width, height;
};

class WaylandWindow {
 public:
  WaylandWindow();
  void create_surface(unsigned width, unsigned height);
  void destroy_surface();
  void toggle_fullscreen();

  WaylandDisplay* display;
  struct geometry geometry, window_size;
  struct wl_egl_window* native;
  struct wl_surface* surface;
  struct wl_shell_surface* shell_surface;
  EGLSurface egl_surface;
  struct wl_callback* callback;
  int fullscreen;
  int configured;
  int opaque;
  void (*drawPtr)(WaylandWindow*);
};

#endif
