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

#include "../common/matrix.h"
#include "../common/display.h"
#include "../common/wayland_platform.h"
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

void redraw(WaylandWindow* window) {
  WaylandPlatform* platform = WaylandPlatform::getInstance();
  static int i = 0;

  ged::Matrix modelview;
  ged::Matrix projection;

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

  float aspect;

  glViewport(0, 0, window->geometry.width, window->geometry.height);

  glClearColor(0.5, 0.5, 0.5, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  
  aspect = window->geometry.width / window->geometry.height;

  //Render a small cube.
  //esFrustum(&perspective, -2.8f, +2.8f, -2.8f * aspect, +2.8f * aspect, 6.0f,
  //          12.0f);
  // Draw a large cube.
  projection.Perspective(29.0f, aspect, 1.0f, 20.0f);
  i++;
  modelview.Translate(0.0f, 0.0f, -8.0f);
  modelview.Rotate(45.0f + (0.25f * i), 1.0f, 0.0f, 0.0f);
  modelview.Rotate(45.0f - (0.5f * i), 0.0f, 1.0f, 0.0f);
  modelview.Rotate(10.0f + (0.15f * i), 0.0f, 0.0f, 1.0f);

 // Compute the final MVP by multiplying the
  // modevleiw and perspective matrices together
  modelview.MatrixMultiply(projection);

  // Load the MVP matrix
  glUniformMatrix4fv(platform->getGL()->mvpLoc, 1, GL_FALSE,
     modelview.Data());

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
}

int main(int argc, char** argv) {
  std::unique_ptr<WaylandPlatform> waylandPlatform = WaylandPlatform::create();
  
  int width = 500;
  int height = 500;
  waylandPlatform->createWindow(width, height, vertexShaderSource,
      fragmentShaderSource, redraw);
  
  // Get the uniform locations
  waylandPlatform->getGL()->mvpLoc = 
       glGetUniformLocation(waylandPlatform->getGL()->program, "u_mvpMatrix");
  waylandPlatform->run();
  waylandPlatform->terminate();

  return 0;
}
