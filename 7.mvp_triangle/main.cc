//
// Triangle example
//
// Shader code came from OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Simple_VertexShader.c

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

void redraw(WaylandWindow* window) {
  WaylandPlatform* platform = WaylandPlatform::getInstance();

  ged::Matrix modelview;
  ged::Matrix projection;

  float vertices[] = {
      0.0f,  0.5f,  0.0f,  // left
      -0.5f, -0.5f, 0.0f,  // right
      0.5f,  -0.5f, 0.0f   // top
  };

  glViewport(0, 0, window->geometry.width, window->geometry.height);

  glClearColor(0.0, 0.0, 0.0, 0.5);
  glClear(GL_COLOR_BUFFER_BIT);
  
  float aspect = window->geometry.width / window->geometry.height;

  // Generate a perspective matrix with a 60 degree FOV.
  float field_of_view = 60.0f;
  projection.Perspective(field_of_view, aspect, 1.0f, 20.0f);
 
  // Translate away from the viewer
  modelview.Translate(0.0, 0.0, -2.0 );

  // Rotate the cube
  GLfloat angle = 60 * M_PI / 180.0;
  modelview.Rotate(angle, 1.0, 0.0, 1.0);

  // Compute the final MVP by multiplying the
  // modevleiw and perspective matrices together
  modelview.MatrixMultiply(projection);

  // Load the MVP matrix
  glUniformMatrix4fv(platform->getGL()->mvpLoc, 1, GL_FALSE,
     modelview.Data());

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
  glEnableVertexAttribArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glDisableVertexAttribArray(0);
}

int main(int argc, char** argv) {
  std::unique_ptr<WaylandPlatform> waylandPlatform = WaylandPlatform::create();
  
  int width = 250;
  int height = 250;
  waylandPlatform->createWindow(width, height, vertexShaderSource,
      fragmentShaderSource, redraw);
  
  // Get the uniform locations
  waylandPlatform->getGL()->mvpLoc = 
       glGetUniformLocation(waylandPlatform->getGL()->program, "u_mvpMatrix");
  waylandPlatform->run();
  waylandPlatform->terminate();

  return 0;
}
