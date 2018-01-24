//
// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
// (c) 2018 Joone Hur
//
// Example_6_6.c
//
//    This example demonstrates drawing a primitive with
//    a separate VBO per attribute.
//
//    Joone makes this demo work in Wayland
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

#define VERTEX_POS_SIZE       3 // x, y and z
#define VERTEX_COLOR_SIZE     4 // r, g, b, and a

#define VERTEX_POS_INDEX       0
#define VERTEX_COLOR_INDEX     1

const char* vertexShaderSource =
      "#version 300 es                            \n"
      "layout(location = 0) in vec4 a_position;   \n"
      "layout(location = 1) in vec4 a_color;      \n"
      "out vec4 v_color;                          \n"
      "void main()                                \n"
      "{                                          \n"
      "    v_color = a_color;                     \n"
      "    gl_Position = a_position;              \n"
      "}";

const char* fragmentShaderSource =
      "#version 300 es            \n"
      "precision mediump float;   \n"
      "in vec4 v_color;           \n"
      "out vec4 o_fragColor;      \n"
      "void main()                \n"
      "{                          \n"
      "    o_fragColor = v_color; \n"
      "}" ;

void redraw(void* data, struct wl_callback* callback, uint32_t time) {
  struct window* window = data;

  // 3 vertices, with (x,y,z) ,(r, g, b, a) per-vertex
  // triangle
  GLfloat vertexPos[3 * VERTEX_POS_SIZE] =
  {
     0.0f,  0.5f, 0.0f,        // v0
     -0.5f, -0.5f, 0.0f,       // v1
     0.5f, -0.5f, 0.0f         // v2
  };

  GLfloat color[4 * VERTEX_COLOR_SIZE] =
  {
     1.0f, 0.0f, 0.0f, 1.0f,   // c0
     0.0f, 1.0f, 0.0f, 1.0f,   // c1
     0.0f, 0.0f, 1.0f, 1.0f    // c2
  };

  GLint vtxStrides[2] =
  {
     VERTEX_POS_SIZE * sizeof ( GLfloat ),
     VERTEX_COLOR_SIZE * sizeof ( GLfloat )
  };

  // Index buffer data
  GLushort indices[3] = { 0, 1, 2 };
  GLfloat *vtxBuf[2] = { vertexPos, color };

  struct wl_region* region;
  assert(window->callback == callback);
  window->callback = NULL;

  if (callback)
    wl_callback_destroy(callback);

  if (!window->configured)
    return;

  glViewport(0, 0, window->geometry.width, window->geometry.height);
  glClear(GL_COLOR_BUFFER_BIT);

  // DrawPrimitiaveWithVBOs
  int numVertices = 3;
  int numIndices = 3;

  if (window->gl.vertexBuffer[0] == 0 &&
      window->gl.vertexBuffer[1] == 0 &&
      window->gl.vertexBuffer[2] == 0) {
    // Only allocate on the first draw
    glGenBuffers(3, window->gl.vertexBuffer);
    // Copy our vertices array in a buffer for GPU to use.
    glBindBuffer(GL_ARRAY_BUFFER, window->gl.vertexBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, vtxStrides[0] * numVertices,
                 vtxBuf[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, window->gl.vertexBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, vtxStrides[1] * numVertices,
                 vtxBuf[1], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window->gl.vertexBuffer[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * numIndices,
                 indices, GL_STATIC_DRAW);
  } 

  glBindBuffer(GL_ARRAY_BUFFER, window->gl.vertexBuffer[0]);
  glEnableVertexAttribArray(VERTEX_POS_INDEX);
  // #define VERTEX_POS_INDEX       0
  // 0 is the location in the vertex shader:
  // layout(location = 0) in vec4 a_position; 
  glVertexAttribPointer(VERTEX_POS_INDEX, VERTEX_POS_SIZE,
                          GL_FLOAT, GL_FALSE, vtxStrides[0], 0 );

  // #define VERTEX_COLOR_INDEX     1
  // 1 is the location in the vertex shader:
  // layout(location = 1) in vec4 a_color;
  glBindBuffer(GL_ARRAY_BUFFER, window->gl.vertexBuffer[1]);
  glEnableVertexAttribArray(VERTEX_COLOR_INDEX);
  glVertexAttribPointer(VERTEX_COLOR_INDEX, VERTEX_COLOR_SIZE,
                        GL_FLOAT, GL_FALSE, vtxStrides[1], 0 );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window->gl.vertexBuffer[2]);

  glDrawElements(GL_TRIANGLES, numIndices,
                 GL_UNSIGNED_SHORT, 0 );

  glDisableVertexAttribArray(VERTEX_POS_INDEX);
  glDisableVertexAttribArray(VERTEX_COLOR_INDEX);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
