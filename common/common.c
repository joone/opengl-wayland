#include <assert.h>
#include <signal.h>
#include <stdio.h>

#include "window.h"

void init_egl(struct display* display, int opaque) {
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

  display->egl.dpy = eglGetDisplay(display->display);
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

void fini_egl(struct display* display) {
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

void init_gl(struct window* window,
             const char* vert_shader_text,
             const char* frag_shader_text) {
  GLuint frag, vert;
  GLuint program;
  GLint status;

  frag = create_shader(frag_shader_text, GL_FRAGMENT_SHADER);
  vert = create_shader(vert_shader_text, GL_VERTEX_SHADER);

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

  window->gl.pos = 0;
  window->gl.col = 1;

  glBindAttribLocation(program, window->gl.pos, "pos");
  glBindAttribLocation(program, window->gl.col, "color");
  glLinkProgram(program);

  // Return the location of a uniform variable.
  window->gl.rotation_uniform = glGetUniformLocation(program, "rotation");
}
