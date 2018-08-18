#include <assert.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "window.h"

static void handle_ping(void* data,
                        struct wl_shell_surface* shell_surface,
                        uint32_t serial) {
  wl_shell_surface_pong(shell_surface, serial);
}

static void handle_configure(void* data,
                             struct wl_shell_surface* shell_surface,
                             uint32_t edges,
                             int32_t width,
                             int32_t height) {
  struct window* window = data;

  if (window->native)
    wl_egl_window_resize(window->native, width, height, 0, 0);

  window->geometry.width = width;
  window->geometry.height = height;

  if (!window->fullscreen)
    window->window_size = window->geometry;
}

static void handle_popup_done(void* data,
                              struct wl_shell_surface* shell_surface) {}

static const struct wl_shell_surface_listener shell_surface_listener = {
    handle_ping, handle_configure, handle_popup_done};

static void configure_callback(void* data,
                               struct wl_callback* callback,
                               uint32_t time) {
  struct window* window = data;

  wl_callback_destroy(callback);

  window->configured = 1;

  if (window->callback == NULL)
    redraw(data, NULL, time);
}

static struct wl_callback_listener configure_callback_listener = {
    configure_callback,
};

static void toggle_fullscreen(struct window* window, int fullscreen) {
  struct wl_callback* callback;

  window->fullscreen = fullscreen;
  window->configured = 0;

  if (fullscreen) {
    wl_shell_surface_set_fullscreen(window->shell_surface,
                                    WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                    0, NULL);
  } else {
    wl_shell_surface_set_toplevel(window->shell_surface);
    handle_configure(window, window->shell_surface, 0,
                     window->window_size.width, window->window_size.height);
  }

  callback = wl_display_sync(window->display->display);
  wl_callback_add_listener(callback, &configure_callback_listener, window);
}

void create_surface(struct window* window) {
  struct display* display = window->display;
  EGLBoolean ret;

  // Ask the wayland compositor to create a surfae for client.
  window->surface = wl_compositor_create_surface(display->compositor);
  window->shell_surface =
      wl_shell_get_shell_surface(display->shell, window->surface);

  wl_shell_surface_add_listener(window->shell_surface, &shell_surface_listener,
                                window);

  window->native = wl_egl_window_create(
      window->surface, window->window_size.width, window->window_size.height);
  window->egl_surface = eglCreateWindowSurface(
      display->egl.dpy, display->egl.conf, window->native, NULL);

  wl_shell_surface_set_title(window->shell_surface, "simple-egl");

  ret = eglMakeCurrent(window->display->egl.dpy, window->egl_surface,
                       window->egl_surface, window->display->egl.ctx);
  assert(ret == EGL_TRUE);

  toggle_fullscreen(window, window->fullscreen);
}

void destroy_surface(struct window* window) {
  /* Required, otherwise segfault in egl_dri2.c: dri2_make_current()
   * on eglReleaseThread(). */
  eglMakeCurrent(window->display->egl.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
                 EGL_NO_CONTEXT);

  eglDestroySurface(window->display->egl.dpy, window->egl_surface);
  wl_egl_window_destroy(window->native);

  wl_shell_surface_destroy(window->shell_surface);
  wl_surface_destroy(window->surface);

  if (window->callback)
    wl_callback_destroy(window->callback);
}

const struct wl_callback_listener frame_listener = {redraw};

static void pointer_handle_enter(void* data,
                                 struct wl_pointer* pointer,
                                 uint32_t serial,
                                 struct wl_surface* surface,
                                 wl_fixed_t sx,
                                 wl_fixed_t sy) {
  struct display* display = data;
  struct wl_buffer* buffer;
  struct wl_cursor* cursor = display->default_cursor;
  struct wl_cursor_image* image;

  if (display->window->fullscreen)
    wl_pointer_set_cursor(pointer, serial, NULL, 0, 0);
  else if (cursor) {
    image = display->default_cursor->images[0];
    buffer = wl_cursor_image_get_buffer(image);
    wl_pointer_set_cursor(pointer, serial, display->cursor_surface,
                          image->hotspot_x, image->hotspot_y);
    wl_surface_attach(display->cursor_surface, buffer, 0, 0);
    wl_surface_damage(display->cursor_surface, 0, 0, image->width,
                      image->height);
    wl_surface_commit(display->cursor_surface);
  }
}

static void pointer_handle_leave(void* data,
                                 struct wl_pointer* pointer,
                                 uint32_t serial,
                                 struct wl_surface* surface) {}

static void pointer_handle_motion(void* data,
                                  struct wl_pointer* pointer,
                                  uint32_t time,
                                  wl_fixed_t sx,
                                  wl_fixed_t sy) {}

static void pointer_handle_button(void* data,
                                  struct wl_pointer* wl_pointer,
                                  uint32_t serial,
                                  uint32_t time,
                                  uint32_t button,
                                  uint32_t state) {
  struct display* display = data;

  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
    wl_shell_surface_move(display->window->shell_surface, display->seat,
                          serial);
}

static void pointer_handle_axis(void* data,
                                struct wl_pointer* wl_pointer,
                                uint32_t time,
                                uint32_t axis,
                                wl_fixed_t value) {}

static const struct wl_pointer_listener pointer_listener = {
    pointer_handle_enter,  pointer_handle_leave, pointer_handle_motion,
    pointer_handle_button, pointer_handle_axis,
};

static void keyboard_handle_keymap(void* data,
                                   struct wl_keyboard* keyboard,
                                   uint32_t format,
                                   int fd,
                                   uint32_t size) {}

static void keyboard_handle_enter(void* data,
                                  struct wl_keyboard* keyboard,
                                  uint32_t serial,
                                  struct wl_surface* surface,
                                  struct wl_array* keys) {}

static void keyboard_handle_leave(void* data,
                                  struct wl_keyboard* keyboard,
                                  uint32_t serial,
                                  struct wl_surface* surface) {}

static void keyboard_handle_key(void* data,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t key,
                                uint32_t state) {
  struct display* d = data;

  if (key == KEY_F11 && state)
    toggle_fullscreen(d->window, d->window->fullscreen ^ 1);
  else if (key == KEY_ESC && state)
    running = 0;
}

static void keyboard_handle_modifiers(void* data,
                                      struct wl_keyboard* keyboard,
                                      uint32_t serial,
                                      uint32_t mods_depressed,
                                      uint32_t mods_latched,
                                      uint32_t mods_locked,
                                      uint32_t group) {}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap, keyboard_handle_enter,     keyboard_handle_leave,
    keyboard_handle_key,    keyboard_handle_modifiers,
};

static void seat_handle_capabilities(void* data,
                                     struct wl_seat* seat,
                                     enum wl_seat_capability caps) {
  struct display* d = data;

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !d->pointer) {
    d->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(d->pointer, &pointer_listener, d);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && d->pointer) {
    wl_pointer_destroy(d->pointer);
    d->pointer = NULL;
  }

  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !d->keyboard) {
    d->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(d->keyboard, &keyboard_listener, d);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && d->keyboard) {
    wl_keyboard_destroy(d->keyboard);
    d->keyboard = NULL;
  }
}

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
};

const struct wl_registry_listener registry_listener = {
    registry_handle_global, registry_handle_global_remove};

void registry_handle_global_remove(void* data,
                                   struct wl_registry* registry,
                                   uint32_t name) {}

void registry_handle_global(void* data,
                            struct wl_registry* registry,
                            uint32_t name,
                            const char* interface,
                            uint32_t version) {
  struct display* d = data;

  if (strcmp(interface, "wl_compositor") == 0) {
    d->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, 1);
  } else if (strcmp(interface, "wl_shell") == 0) {
    d->shell = wl_registry_bind(registry, name, &wl_shell_interface, 1);
  } else if (strcmp(interface, "wl_seat") == 0) {
    d->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    wl_seat_add_listener(d->seat, &seat_listener, d);
  } else if (strcmp(interface, "wl_shm") == 0) {
    d->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    d->cursor_theme = wl_cursor_theme_load(NULL, 32, d->shm);
    d->default_cursor = wl_cursor_theme_get_cursor(d->cursor_theme, "left_ptr");
  }
}

void signal_int(int signum) {
  running = 0;
}

void usage(int error_code) {
  fprintf(stderr,
          "Usage: simple-egl [OPTIONS]\n\n"
          "  -f\tRun in fullscreen mode\n"
          "  -o\tCreate an opaque surface\n"
          "  -h\tThis help text\n\n");

  exit(error_code);
}
