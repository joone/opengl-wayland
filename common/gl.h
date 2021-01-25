// (c) 2018-2021 Joone Hur
//

#ifndef OPENGL_WAYLAND_GL_H_
#define OPENGL_WAYLAND_GL_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <vector>
#include "window.h"

class GL {
 public:
  void init_gl(unsigned width, unsigned height, 
      const char* vertShaderText, const char* fragShaderText);
  void init_egl(WaylandDisplay* display, int opaque);
  void finish_egl(WaylandDisplay* display);
  unsigned getViewportWidth() { return viewportWidth_; }
  unsigned getViewportHeight() { return viewportHeight_; }

// private:
  GLuint program;
  GLuint rotation_uniform;
  GLuint pos;
  GLuint col;
  GLuint texture_id;  // Texture handle.
  GLint sampler;      // Sampler location.
	GLuint vertexBuffer[3];   // Vertex Buffer.
  GLuint mvpLoc; // Uniform location.
  ESMatrix mvpMatrix;

 private:
  unsigned viewportWidth_;
  unsigned viewportHeight_;

};

#endif
