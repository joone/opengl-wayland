/*
 * Copyright © 2011 Benjamin Franzke
 * Copyright © 2018 Joone Hur
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

// #version 300 es
const char* vertexShaderSource =
    "#version 300 es                            \n"
    "uniform mat4 u_mvpMatrix;                  \n"
    "layout(location = 0) in vec4 pos;          \n"
    "layout(location = 1) in vec4 color;        \n"
    "out vec4 vVaryingColor;                \n"
    "void main()                                \n"
    "{                                          \n"
    "  gl_Position = u_mvpMatrix * pos;         \n"
    "  vVaryingColor = color;                   \n"
    "}";

const char* fragmentShaderSource =
    "#version 300 es                                 \n"
    "precision mediump float;                        \n"
    "layout(location = 0) out vec4 out_color;        \n"
    "in lowp vec4 vVaryingColor;                    \n"
    "void main()                                     \n"
    "{                                               \n"
    "   out_color = vVaryingColor;                   \n"
    "}                                               \n";

void redraw(void* data, struct wl_callback* callback, uint32_t time) {
  struct window* window = data;
  static i = 0;

  ESMatrix perspective;
  ESMatrix modelview;

 static const GLfloat vertices[] = {
      // front
      -1.0f, -1.0f, +1.0f, // point blue
      +1.0f, -1.0f, +1.0f, // point magenta
      -1.0f, +1.0f, +1.0f, // point cyan
      +1.0f, +1.0f, +1.0f, // point white
      // back
      +1.0f, -1.0f, -1.0f, // point red
      -1.0f, -1.0f, -1.0f, // point black
      +1.0f, +1.0f, -1.0f, // point yellow
      -1.0f, +1.0f, -1.0f, // point green
      // right
      +1.0f, -1.0f, +1.0f, // point magenta
      +1.0f, -1.0f, -1.0f, // point red
      +1.0f, +1.0f, +1.0f, // point white
      +1.0f, +1.0f, -1.0f, // point yellow
      // left
      -1.0f, -1.0f, -1.0f, // point black
      -1.0f, -1.0f, +1.0f, // point blue
      -1.0f, +1.0f, -1.0f, // point green
      -1.0f, +1.0f, +1.0f, // point cyan
      // top
      -1.0f, +1.0f, +1.0f, // point cyan
      +1.0f, +1.0f, +1.0f, // point white
      -1.0f, +1.0f, -1.0f, // point green
      +1.0f, +1.0f, -1.0f, // point yellow
      // bottom
      -1.0f, -1.0f, -1.0f, // point black
      +1.0f, -1.0f, -1.0f, // point red
      -1.0f, -1.0f, +1.0f, // point blue
      +1.0f, -1.0f, +1.0f  // point magenta
  };

  static const GLfloat vColors[] = {
      // frontL white
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      // back: red
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      // right
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      // left
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      // top: yellow
      1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
      // bottom: purple
      1.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 1.0f
  };

  struct wl_region* region;
  assert(window->callback == callback);
  window->callback = NULL;

  float aspect;

  if (callback)
    wl_callback_destroy(callback);

  if (!window->configured)
    return;

  glViewport(0, 0, window->geometry.width, window->geometry.height);

  glClearColor(0.5, 0.5, 0.5, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  
  aspect = window->geometry.width / window->geometry.height;

  // Generate a perspective matrix with a 60 degree FOV.
  esMatrixLoadIdentity(&perspective);
  //Render a small cube.
  //esFrustum(&perspective, -2.8f, +2.8f, -2.8f * aspect, +2.8f * aspect, 6.0f,
  //          12.0f);
  // Draw a large cube.
  esPerspective(&perspective, 29.0f, aspect, 1.0f, 20.0f);
  i++;
  esMatrixLoadIdentity(&modelview);
  esTranslate(&modelview, 0.0f, 0.0f, -8.0f);
  esRotate(&modelview, 45.0f + (0.25f * i), 1.0f, 0.0f, 0.0f);
  esRotate(&modelview, 45.0f - (0.5f * i), 0.0f, 1.0f, 0.0f);
  esRotate(&modelview, 10.0f + (0.15f * i), 0.0f, 0.0f, 1.0f);

  esMatrixMultiply(&window->gl.mvpMatrix, &modelview, &perspective);

  // Load the MVP matrix
  glUniformMatrix4fv(window->gl.mvpLoc, 1, GL_FALSE,
     (GLfloat*)&window->gl.mvpMatrix.m[0][0]);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
  glEnableVertexAttribArray(0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
  glDisableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, vColors);
  glEnableVertexAttribArray(1);

  wl_surface_set_opaque_region(window->surface, NULL);
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
  window.window_size.width = 500;
  window.window_size.height = 500;

  display.display = wl_display_connect(NULL);
  assert(display.display);

  display.registry = wl_display_get_registry(display.display);
  wl_registry_add_listener(display.registry, &registry_listener, &display);

  wl_display_dispatch(display.display);

  init_egl(&display, window.opaque);
  create_surface(&window);
  init_gl(&window, vertexShaderSource, fragmentShaderSource);

  // Get the uniform locations
  window.gl.mvpLoc = glGetUniformLocation(window.gl.program, "u_mvpMatrix");
  display.cursor_surface = wl_compositor_create_surface(display.compositor);

  sigint.sa_handler = signal_int;
  sigemptyset(&sigint.sa_mask);
  sigint.sa_flags = SA_RESETHAND;
  sigaction(SIGINT, &sigint, NULL);

  while (running && ret != -1)
    ret = wl_display_dispatch(display.display);

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
