/*
 * Copyright © 2011 Benjamin Franzke
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

#include <chrono>
#include <ctime>
#include <iostream>
//#include <sys/time.h>

#include "../common/wayland_platform.h"
#include "../common/display.h"
#include "../common/window.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

// The main purpose of the vertex shader is to transform 3D coordinates
// into different 3D coordinates (more on that later) and the vertex shader
// allows us to do some basic processing on the vertex attributes.
// https://learnopengl.com/#!Getting-started/Hello-Triangle
const char* vert_shader_text =
    "uniform mat4 rotation;\n"
    "attribute vec4 pos;\n"
    "attribute vec4 color;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_Position = rotation * pos;\n"
    "  v_color = color;\n"
    "}\n";

const char* frag_shader_text =
    "precision mediump float;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_FragColor = v_color;\n"
    "}\n";

void redraw(WaylandWindow* window) {
  WaylandPlatform* platform = WaylandPlatform::getInstance();
  static const GLfloat verts[3][2] = {{-0.5, -0.5}, {0.5, -0.5}, {0, 0.5}};
  static const GLfloat colors[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  GLfloat angle;
  GLfloat rotation[4][4] = {
      {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
  static const int32_t speed_div = 5;
  static auto start_time = 0;

  struct wl_region* region;

  if (start_time == 0)
    start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  auto cur_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

  std::cout << "start time = " << start_time << std::endl;
  std::cout << "cur_time = " << cur_time << std::endl; 
  angle = ((cur_time - start_time) / speed_div) % 360 * M_PI / 180.0;
  std::cout << "angle=" << angle << std::endl;
  rotation[0][0] = cos(angle);
  rotation[0][2] = sin(angle);
  rotation[2][0] = -sin(angle);
  rotation[2][2] = cos(angle);

  glViewport(0, 0, window->geometry.width, window->geometry.height);

  glUniformMatrix4fv(platform->getGL()->rotation_uniform, 1, GL_FALSE,
                     (GLfloat*)rotation);

  glClearColor(0.0, 0.0, 0.0, 0.5);
  glClear(GL_COLOR_BUFFER_BIT);

  glVertexAttribPointer(platform->getGL()->pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  glVertexAttribPointer(platform->getGL()->col, 3, GL_FLOAT, GL_FALSE, 0, colors);
  glEnableVertexAttribArray(platform->getGL()->pos);
  glEnableVertexAttribArray(platform->getGL()->col);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glDisableVertexAttribArray(platform->getGL()->pos);
  glDisableVertexAttribArray(platform->getGL()->col);
}

int main(int argc, char** argv) {
  std::unique_ptr<WaylandPlatform> waylandPlatform = WaylandPlatform::create();
  
  int width = 250;
  int height = 250;

  /*for (i = 1; i < argc; i++) {
    if (strcmp("-f", argv[i]) == 0)
      window.fullscreen = 1;
    else if (strcmp("-o", argv[i]) == 0)
      window.opaque = 1;
    else if (strcmp("-h", argv[i]) == 0)
      usage(EXIT_SUCCESS);
    else
      usage(EXIT_FAILURE);
  }*/

  waylandPlatform->createWindow(width, height,vert_shader_text,
      frag_shader_text, redraw);

  waylandPlatform->run();
  waylandPlatform->terminate();

  return 0;
}
