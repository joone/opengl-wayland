#ifndef OPENGL_WAYLAND_PLATFORM_H_
#define OPENGL_WAYLAND_PLATFORM_H_

#include <functional>
#include "memory"
#include "gl.h"

class WaylandPlatform {
 public:

  WaylandPlatform(const WaylandPlatform&) = delete;
  void operator=(const WaylandPlatform&) = delete;

  ~WaylandPlatform();

  static std::unique_ptr<WaylandPlatform> create();
   
  bool initialize();
  void createWindow(unsigned width, unsigned height,
      const char* vertShaderText, const char* fragShaderText,
      void (*drawPtr)(WaylandWindow*));
  static WaylandPlatform* getInstance();
  void initGL();
  void run();
  void terminate();
  GL* getGL() { return gl_.get(); }

 private:
  WaylandPlatform();
  std::unique_ptr<WaylandDisplay> display_;
  std::unique_ptr<GL> gl_;
};

#endif
