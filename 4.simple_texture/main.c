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

//
// This is a simple example that draws a quad with a 2D
// texture image. The purpose of this example is to demonstrate
// the basics of 2D texturing.
//
// Some of code comes from the below example:
// https://github.com/danginsburg/opengles3-book/blob/master/Chapter_9/Simple_Texture2D/Simple_Texture2D.c

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
    "#version 300 es                            \n"
    "layout(location = 0) in vec4 a_position;   \n"
    "layout(location = 1) in vec2 a_texCoord;   \n"
    "out vec2 v_texCoord;                       \n"
    "void main()                                \n"
    "{                                          \n"
    "   gl_Position = a_position;               \n"
    "   v_texCoord = a_texCoord;                \n"
    "}                                          \n";

const char* frag_shader_text =
    "#version 300 es                                     \n"
    "precision mediump float;                            \n"
    "in vec2 v_texCoord;                                 \n"
    "layout(location = 0) out vec4 outColor;             \n"
    "uniform sampler2D s_texture;                        \n"
    "void main()                                         \n"
    "{                                                   \n"
    "  outColor = texture( s_texture, v_texCoord );      \n"
    "}                                                   \n";

GLuint CreateSimpleTexture2D() {
  // Texture object handle
  GLuint textureId;

  // 2x2 Image, 3 bytes per pixel (R, G, B)
  GLubyte pixels[4 * 3] = {
      255, 0,   0,    // Red
      0,   255, 0,    // Green
      0,   0,   255,  // Blue
      255, 255, 0     // Yellow
  };

  // When texture data is uploaded via glTexImage2D, the rows of pixels are
  // assumed to be aligned to the value set for GL_UNPACK_ALIGNMENT.
  // Use tightly packed data.
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Generate a texture object
  glGenTextures(1, &textureId);

  // Bind the texture object
  glBindTexture(GL_TEXTURE_2D, textureId);

  // Load the texture
  // target
  // level
  // internalFormat
  // width: the width of the image in pixels.
  // height:
  // border: ignored in OpenGL ES.
  // format:
  // type: the type of the incoming pixel data.
  // pixels: contains the actual pixel data for the image.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE,
               pixels);

  // Set the minification and magnification filtering mode.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return textureId;
}

void redraw(void* data, struct wl_callback* callback, uint32_t time) {
  struct window* window = data;
  GLfloat vVertices[] = {
      -0.5f, 0.5f,  0.0f,  // Position 0
      0.0f,  0.0f,         // TexCoord 0
      -0.5f, -0.5f, 0.0f,  // Position 1
      0.0f,  1.0f,         // TexCoord 1
      0.5f,  -0.5f, 0.0f,  // Position 2
      1.0f,  1.0f,         // TexCoord 2
      0.5f,  0.5f,  0.0f,  // Position 3
      1.0f,  0.0f          // TexCoord 3
  };

  // The below sets of indices have the same result:
  // GLushort indices[] = {0, 1, 2, 0, 2, 3};
  // GLushort indices[] = {0, 2, 3, 0, 1, 2};
  GLushort indices[] = {1 ,2, 0, 2, 3, 0};
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

  // Load the vertex position
  // index
  // size: the number of components per generic vertext attribute.
  // type
  // normalized
  // stride: specified the byte offset between cosecutive gnertic vertext
  //         attributes.
  // pointer: specifies a offset of the first component of the first generic
  //          vertex attributes.

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                        vVertices);
  // Load the texture coordinate
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                        &vVertices[3]);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // Bind the texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, window->gl.texture_id);
  // Set the sampler texture unit to 0
  glUniform1i(window->gl.sampler, 0);

  // Render primitives from array data.
  // mode: specifies what kind of primitive to render.
  // count: specifies the number of elements to be rendered.
  // type: specifies the type of the value in indices.
  // indices: specifies the type of the values in indices.
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

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
  window.gl.texture_id = CreateSimpleTexture2D();

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
