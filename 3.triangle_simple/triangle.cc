//
// Triangle example
// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/1.getting_started/2.1.hello_triangle/hello_triangle.cpp
//

#include "../common/wayland_platform.h"
#include "../common/display.h"
#include "../common/window.h"

// #version 300 es
const char* vertexShaderSource =
    "#version 300 es                          \n"
    "layout(location = 0) in vec4 position;   \n"
    "void main()                              \n"
    "{                                        \n"
    "   gl_Position = position;  \n"
    "}";

const char* fragmentShaderSource =
    "#version 300 es                                 \n"
    "precision mediump float;                        \n"
    "layout(location = 0) out vec4 out_color;    \n"
    "void main()                                     \n"
    "{                                               \n"
    "   out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f); \n"
    "}                                               \n";

void redraw(WaylandWindow* window) {
  float vertices[] = {
      0.0f,  0.5f,  0.0f,  // left
      -0.5f, -0.5f, 0.0f,  // right
      0.5f,  -0.5f, 0.0f   // top
  };

  WaylandPlatform* platform = WaylandPlatform::getInstance();

  glViewport(0, 0, window->geometry.width, window->geometry.height);

  glClearColor(0.0, 0.0, 0.0, 0.5);
  glClear(GL_COLOR_BUFFER_BIT);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
  glEnableVertexAttribArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glDisableVertexAttribArray(0);
}

int main(int argc, char** argv) {
  std::unique_ptr<WaylandPlatform> waylandPlatform = WaylandPlatform::create();
 
  int width = 250;
  int height = 250;

  waylandPlatform->createWindow(width, height,vertexShaderSource,
      fragmentShaderSource, redraw);

  waylandPlatform->run();
  waylandPlatform->terminate();

  return 0;
}
