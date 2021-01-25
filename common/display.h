#ifndef OPENGL_WAYLAND_DISPLAY_H_
#define OPENGL_WAYLAND_DISPLAY_H_

#include <memory>

#include <EGL/egl.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>

class WaylandWindow;

class WaylandDisplay {
 public:
  WaylandDisplay();
  void InitializeDisplay();
  void CreateAcceleratedSurface(unsigned width, unsigned height);
  WaylandWindow* GetWindow();
  void Terminate();
  void Run();
  static void registry_handle_global(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);
  static void registry_handle_global_remove(
      void* data,
      struct wl_registry* registry,
      uint32_t name);

  struct wl_display* display_;
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

 private:
   // FIXME: will support multiple windows.
   std::unique_ptr<WaylandWindow> window_;
};

#endif