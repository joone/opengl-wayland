#ifndef COMMON_H_
#define COMMON_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "window.h"

void init_egl(struct display* display, int opaque);
void fini_egl(struct display* display);
void init_gl(struct window* window,
             const char* vert_shader_text,
             const char* frag_shader_text);

#endif
