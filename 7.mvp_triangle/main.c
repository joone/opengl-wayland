//
// Triangle example
//

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
    "out vec4 v_color;                          \n"
    "void main()                                \n"
    "{                                          \n"
    "  v_color = color;                         \n"
    "  gl_Position = u_mvpMatrix * pos;         \n"
    "}";

const char* fragmentShaderSource =
    "#version 300 es                                 \n"
    "precision mediump float;                        \n"
    "layout(location = 0) out vec4 out_color;        \n"
    "void main()                                     \n"
    "{                                               \n"
    "   out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);    \n"
    "}                                               \n";

void redraw(void* data, struct wl_callback* callback, uint32_t time) {
  struct window* window = data;

  ESMatrix perspective;
  ESMatrix modelview;

  float vertices[] = {
      0.0f,  0.5f,  0.0f,  // left
      -0.5f, -0.5f, 0.0f,  // right
      0.5f,  -0.5f, 0.0f   // top
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

  glClearColor(0.0, 0.0, 0.0, 0.5);
  glClear(GL_COLOR_BUFFER_BIT);
  
  aspect = window->geometry.width / window->geometry.height;

  // Generate a perspective matrix with a 60 degree FOV.
  esMatrixLoadIdentity(&perspective );
  esPerspective(&perspective, 60.0f, aspect, 1.0f, 20.0f);

  // Generate a model view matrix to rotate/translate the triangle. 
  esMatrixLoadIdentity(&modelview);

  // Translate away from the viewer
  esTranslate(&modelview, 0.0, 0.0, -2.0 );

  // Rotate the cube
  GLfloat angle = 60 * M_PI / 180.0;
  esRotate(&modelview, angle, 1.0, 0.0, 1.0);

  // Compute the final MVP by multiplying the
  // modevleiw and perspective matrices together
  esMatrixMultiply(&window->gl.mvpMatrix, &modelview, &perspective);

  // Load the MVP matrix
  glUniformMatrix4fv(window->gl.mvpLoc, 1, GL_FALSE,
     (GLfloat*)&window->gl.mvpMatrix.m[0][0]);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
  glEnableVertexAttribArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glDisableVertexAttribArray(0);

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
  window.window_size.width = 250;
  window.window_size.height = 250;

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
