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

#include "../common/display.h"
#include "../common/wayland_platform.h"
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

void redraw(WaylandWindow* window) {
  WaylandPlatform* platform = WaylandPlatform::getInstance();

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

  glViewport(0, 0, window->geometry.width, window->geometry.height);
  glClear(GL_COLOR_BUFFER_BIT);

  // DrawPrimitiaveWithVBOs
  int numVertices = 3;
  int numIndices = 3;

  if (platform->getGL()->vertexBuffer[0] == 0 &&
      platform->getGL()->vertexBuffer[1] == 0 &&
      platform->getGL()->vertexBuffer[2] == 0) {
    // Only allocate on the first draw
    glGenBuffers(3, platform->getGL()->vertexBuffer);
    // Copy our vertices array in a buffer for GPU to use.
    glBindBuffer(GL_ARRAY_BUFFER, platform->getGL()->vertexBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, vtxStrides[0] * numVertices,
                 vtxBuf[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, platform->getGL()->vertexBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, vtxStrides[1] * numVertices,
                 vtxBuf[1], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, platform->getGL()->vertexBuffer[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * numIndices,
                 indices, GL_STATIC_DRAW);
  } 

  glBindBuffer(GL_ARRAY_BUFFER, platform->getGL()->vertexBuffer[0]);
  glEnableVertexAttribArray(VERTEX_POS_INDEX);
  // #define VERTEX_POS_INDEX       0
  // 0 is the location in the vertex shader:
  // layout(location = 0) in vec4 a_position; 
  glVertexAttribPointer(VERTEX_POS_INDEX, VERTEX_POS_SIZE,
                          GL_FLOAT, GL_FALSE, vtxStrides[0], 0 );

  // #define VERTEX_COLOR_INDEX     1
  // 1 is the location in the vertex shader:
  // layout(location = 1) in vec4 a_color;
  glBindBuffer(GL_ARRAY_BUFFER, platform->getGL()->vertexBuffer[1]);
  glEnableVertexAttribArray(VERTEX_COLOR_INDEX);
  glVertexAttribPointer(VERTEX_COLOR_INDEX, VERTEX_COLOR_SIZE,
                        GL_FLOAT, GL_FALSE, vtxStrides[1], 0 );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, platform->getGL()->vertexBuffer[2]);

  glDrawElements(GL_TRIANGLES, numIndices,
                 GL_UNSIGNED_SHORT, 0 );

  glDisableVertexAttribArray(VERTEX_POS_INDEX);
  glDisableVertexAttribArray(VERTEX_COLOR_INDEX);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

int main(int argc, char** argv) {
  std::unique_ptr<WaylandPlatform> waylandPlatform = WaylandPlatform::create();
  
  int width = 250;
  int height = 250;
  waylandPlatform->createWindow(width, height, vertexShaderSource,
      fragmentShaderSource, redraw);

  waylandPlatform->run();
  waylandPlatform->terminate();
 
  return 0;
}
