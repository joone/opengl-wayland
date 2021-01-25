#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "gl.h"
#include "window.h"
#include "wayland_platform.h"    

void GL::init_egl(WaylandDisplay* display, int opaque) {
  static const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                           EGL_NONE};

  EGLint config_attribs[] = {EGL_SURFACE_TYPE,
                             EGL_WINDOW_BIT,
                             EGL_RED_SIZE,
                             1,
                             EGL_GREEN_SIZE,
                             1,
                             EGL_BLUE_SIZE,
                             1,
                             EGL_ALPHA_SIZE,
                             1,
                             EGL_RENDERABLE_TYPE,
                             EGL_OPENGL_ES2_BIT,
                             EGL_NONE};

  EGLint major, minor, n;
  EGLBoolean ret;

  if (opaque)
    config_attribs[9] = 0;

  display->egl.dpy = eglGetDisplay((EGLNativeDisplayType)display->display_);
  assert(display->egl.dpy);

  ret = eglInitialize(display->egl.dpy, &major, &minor);
  assert(ret == EGL_TRUE);
  ret = eglBindAPI(EGL_OPENGL_ES_API);
  assert(ret == EGL_TRUE);

  ret = eglChooseConfig(display->egl.dpy, config_attribs, &display->egl.conf, 1,
                        &n);
  assert(ret && n == 1);

  display->egl.ctx = eglCreateContext(display->egl.dpy, display->egl.conf,
                                      EGL_NO_CONTEXT, context_attribs);
  assert(display->egl.ctx);
}

void GL::finish_egl(WaylandDisplay* display) {
  eglTerminate(display->egl.dpy);
  eglReleaseThread();
}

static GLuint create_shader(const char* source, GLenum shader_type) {
  GLuint shader;
  GLint status;

  shader = glCreateShader(shader_type);
  assert(shader != 0);

  glShaderSource(shader, 1, (const char**)&source, NULL);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (!status) {
    char log[1000];
    GLsizei len;
    glGetShaderInfoLog(shader, 1000, &len, log);
    fprintf(stderr, "Error: compiling %s: %*s\n",
            shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment", len, log);
    exit(1);
  }

  return shader;
}

void GL::init_gl(unsigned width, unsigned height,
    const char* vertShaderText, const char* fragShaderText) {
  GLuint frag, vert;
  GLint status;

  viewportWidth_ = width;
  viewportHeight_ = height;

  frag = create_shader(fragShaderText, GL_FRAGMENT_SHADER);
  vert = create_shader(vertShaderText, GL_VERTEX_SHADER);

  program = glCreateProgram();
  glAttachShader(program, frag);
  glAttachShader(program, vert);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (!status) {
    char log[1000];
    GLsizei len;
    glGetProgramInfoLog(program, 1000, &len, log);
    fprintf(stderr, "Error: linking:\n%*s\n", len, log);
    exit(1);
  }

  glUseProgram(program);

  pos = 0;
  col = 1;

  // VertexBufferObject IDs
  vertexBuffer[0] = 0;
  vertexBuffer[1] = 0;
  vertexBuffer[2] = 0;

  glBindAttribLocation(program, pos, "pos");
  glBindAttribLocation(program, col, "color");
  glLinkProgram(program);

  // Return the location of a uniform variable.
  rotation_uniform = glGetUniformLocation(program, "rotation");
}
