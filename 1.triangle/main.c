/*
 * Copyright © 2011 Benjamin Franzke
 * Copyright © 2017 Joone Hur
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"
#include "../common/window.h"

const char* vert_shader_text =
    "attribute vec4 pos;\n"
    "void main() {\n"
    "  gl_Position = pos;\n"
    "}\n";

const char* frag_shader_text =
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

void redraw(void* data, struct wl_callback* callback, uint32_t time) {
  struct window* window = data;
  static const GLfloat verts[] = {0.0f, 0.5f, 0.0f,  -0.5, -0.5f,
                                  0.0f, 0.5f, -0.5f, 0.0f};

  struct wl_region* region;

  assert(window->callback == callback);
  window->callback = NULL;

  if (callback)
    wl_callback_destroy(callback);

  if (!window->configured)
    return;

  // Set the viewport.
  glViewport(0, 0, window->geometry.width, window->geometry.height);

  // Clear the color buffer.
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  // Load the vertex data.
  // window->gl.pos = 0
  // The pos variable is bounded to attribue location 0.
  // Call glVertextAttribPointer to load the data into vertex attribue 0.
  glVertexAttribPointer(window->gl.pos, 3, GL_FLOAT, GL_FALSE, 0, verts);
  glEnableVertexAttribArray(window->gl.pos);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glDisableVertexAttribArray(window->gl.pos);

  window->callback = wl_surface_frame(window->surface);
  wl_callback_add_listener(window->callback, &frame_listener, window);

  eglSwapBuffers(window->display->egl.dpy, window->egl_surface);
}

int main(int argc, char** argv) {
  struct sigaction sigint;
  struct display display = {0};
  struct window window = {0};
  int i, ret = 0;

  window.display = &display;
  display.window = &window;
  window.window_size.width = 250;
  window.window_size.height = 250;

  display.display = wl_display_connect(NULL);
  assert(display.display);

  display.registry = wl_display_get_registry(display.display);
  wl_registry_add_listener(display.registry, &registry_listener, &display);

  wl_display_dispatch(display.display);

  init_egl(&display, window.opaque);
  create_surface(&window);
  init_gl(&window, vert_shader_text, frag_shader_text);

  display.cursor_surface = wl_compositor_create_surface(display.compositor);

  sigint.sa_handler = signal_int;
  sigemptyset(&sigint.sa_mask);
  sigint.sa_flags = SA_RESETHAND;
  sigaction(SIGINT, &sigint, NULL);

  while (running && ret != -1)
    ret = wl_display_dispatch(display.display);

  fprintf(stderr, "simple-egl exiting\n");

  destroy_surface(&window);
  fini_egl(&display);

  wl_surface_destroy(display.cursor_surface);
  if (display.cursor_theme)
    wl_cursor_theme_destroy(display.cursor_theme);

  if (display.shell)
    wl_shell_destroy(display.shell);

  if (display.compositor)
    wl_compositor_destroy(display.compositor);

  wl_registry_destroy(display.registry);
  wl_display_flush(display.display);
  wl_display_disconnect(display.display);

  return 0;
}
