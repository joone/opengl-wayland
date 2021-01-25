#include <assert.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "window.h"
#include "wayland_platform.h"

#include "gl.h"

void redraw(void* data, struct wl_callback* callback, unsigned int time);

const struct wl_callback_listener frame_listener = {redraw};

void redraw(void* data, struct wl_callback* callback, unsigned int time) {
  printf("redraw\n");
  WaylandWindow* window = static_cast<WaylandWindow*>(data);
  struct wl_region* region;

  assert(window->callback == callback);
  window->callback = NULL;

  if (callback)
    wl_callback_destroy(callback);

  if (!window->configured)
    return;

  window->drawPtr(window);


  if (window->opaque || window->fullscreen) {
    region = wl_compositor_create_region(window->display->compositor);
    wl_region_add(region, 0, 0, window->geometry.width,
                  window->geometry.height);
    wl_surface_set_opaque_region(window->surface, region);
    wl_region_destroy(region);
  } else {
    wl_surface_set_opaque_region(window->surface, NULL);
  }

  window->callback = wl_surface_frame(window->surface);
  wl_callback_add_listener(window->callback, &frame_listener, window);

  eglSwapBuffers(window->display->egl.dpy, window->egl_surface);
}

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
  WaylandWindow* window = static_cast<WaylandWindow*>(data);

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
  WaylandWindow* window = static_cast<WaylandWindow*>(data);

  wl_callback_destroy(callback);

  window->configured = 1;

  if (window->callback == NULL)
    redraw(data, NULL, time);
}

static struct wl_callback_listener configure_callback_listener = {
    configure_callback,
};

WaylandWindow::WaylandWindow() : fullscreen(1), callback(nullptr) {

}

void WaylandWindow::toggle_fullscreen() {
  struct wl_callback* callback;

  fullscreen = fullscreen^1;
  configured = 0;

  if (fullscreen) {
    wl_shell_surface_set_fullscreen(shell_surface,
                                    WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                    0, NULL);
  } else {
    wl_shell_surface_set_toplevel(shell_surface);
    handle_configure(this, shell_surface, 0,
                     window_size.width, window_size.height);
  }

  callback = wl_display_sync(display->display_);
  wl_callback_add_listener(callback, &configure_callback_listener, this);
}

void WaylandWindow::create_surface(unsigned width, unsigned height) {
  EGLBoolean ret;

  window_size.width = width;
  window_size.height = height;

  surface = wl_compositor_create_surface(display->compositor);
  shell_surface =
      wl_shell_get_shell_surface(display->shell, surface);

  wl_shell_surface_add_listener(shell_surface, &shell_surface_listener,
                                this);

  native = wl_egl_window_create(
      surface, window_size.width, window_size.height);
  egl_surface = eglCreateWindowSurface(
     display->egl.dpy, display->egl.conf, (EGLNativeWindowType)native, NULL);

  wl_shell_surface_set_title(shell_surface, "simple-egl");

  ret = eglMakeCurrent(display->egl.dpy, egl_surface,
                       egl_surface, display->egl.ctx);
  assert(ret == EGL_TRUE);

  toggle_fullscreen();
}

void WaylandWindow::destroy_surface() {
  /* Required, otherwise segfault in egl_dri2.c: dri2_make_current()
   * on eglReleaseThread(). */
  eglMakeCurrent(display->egl.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
                 EGL_NO_CONTEXT);

  eglDestroySurface(display->egl.dpy, egl_surface);
  wl_egl_window_destroy(native);

  wl_shell_surface_destroy(shell_surface);
  wl_surface_destroy(surface);

  if (callback)
    wl_callback_destroy(callback);
}

void usage(int error_code) {
  fprintf(stderr,
          "Usage: simple-egl [OPTIONS]\n\n"
          "  -f\tRun in fullscreen mode\n"
          "  -o\tCreate an opaque surface\n"
          "  -h\tThis help text\n\n");

  exit(error_code);
}
