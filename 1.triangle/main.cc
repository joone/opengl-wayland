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

#include "../common/wayland_platform.h"
#include "../common/display.h"
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

void redraw(WaylandWindow* window) {
  static const GLfloat verts[] = {0.0f, 0.5f, 0.0f,  -0.5, -0.5f,
                                  0.0f, 0.5f, -0.5f, 0.0f};

  WaylandPlatform* platform = WaylandPlatform::getInstance();

  // Set the viewport.
  glViewport(0, 0, window->geometry.width, window->geometry.height);

  // Clear the color buffer.
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  // Load the vertex data.
  // window->gl.pos = 0
  // The pos variable is bounded to attribue location 0.
  // Call glVertextAttribPointer to load the data into vertex attribue 0.
  glVertexAttribPointer(platform->getGL()->pos, 3, GL_FLOAT, GL_FALSE, 0, verts);
  glEnableVertexAttribArray(platform->getGL()->pos);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glDisableVertexAttribArray(platform->getGL()->pos);
}

int main(int argc, char** argv) {
  std::unique_ptr<WaylandPlatform> waylandPlatform = WaylandPlatform::create();
  
  int width = 250;
  int height = 250;
  waylandPlatform->createWindow(width, height,vert_shader_text,
      frag_shader_text, redraw);

  waylandPlatform->run();
  waylandPlatform->terminate();
  return 0;
}
