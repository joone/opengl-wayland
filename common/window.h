#ifndef WINDOW_H_
#define WINDOW_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>

struct window;
struct seat;

struct display {
  struct wl_display* display;
  struct wl_registry* registry;
  struct wl_compositor* compositor;
  struct wl_shell* shell;
  struct wl_seat* seat;
  struct wl_pointer* pointer;
  struct wl_keyboard* keyboard;
  struct wl_shm* shm;
  struct wl_cursor_theme* cursor_theme;
  struct wl_cursor* default_cursor;
  struct wl_surface* cursor_surface;
  struct {
    EGLDisplay dpy;
    EGLContext ctx;
    EGLConfig conf;
  } egl;
  struct window* window;
};

struct geometry {
  int width, height;
};

struct window {
  struct display* display;
  struct geometry geometry, window_size;
  struct {
    GLuint rotation_uniform;
    GLuint pos;
    GLuint col;
    GLuint texture_id;  // Texture handle.
    GLint sampler;      // Sampler location.
  } gl;

  struct wl_egl_window* native;
  struct wl_surface* surface;
  struct wl_shell_surface* shell_surface;
  EGLSurface egl_surface;
  struct wl_callback* callback;
  int fullscreen, configured, opaque;
};

void create_surface(struct window* window);
void destroy_surface(struct window* window);
void registry_handle_global_remove(void* data,
                                   struct wl_registry* registry,
                                   uint32_t name);
void registry_handle_global(void* data,
                            struct wl_registry* registry,
                            uint32_t name,
                            const char* interface,
                            uint32_t version);
void signal_int(int signum);
void redraw(void* data, struct wl_callback* callback, uint32_t time);
void usage(int error_code);

const struct wl_callback_listener frame_listener;
const struct wl_registry_listener registry_listener;
static int running = 1;

#endif
