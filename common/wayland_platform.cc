#include <assert.h>
#include <iostream>
#include <signal.h>

#include "wayland_platform.h"

WaylandPlatform* g_instance = nullptr;

static void signal_int(int signum) {
  running = 0;
}

WaylandPlatform::WaylandPlatform() {
  if (g_instance) 
    std::cout << "There should only be a single WaylandPlatform.";
  g_instance = this;
}

WaylandPlatform::~WaylandPlatform() {
  // terminate();
}

std::unique_ptr<WaylandPlatform> WaylandPlatform::create() {
  std::unique_ptr<WaylandPlatform> backend(new WaylandPlatform());
  if (backend->initialize())
    return backend;
  return nullptr;
}

WaylandPlatform* WaylandPlatform::getInstance() {
  return g_instance;
}

bool WaylandPlatform::initialize() {
  display_ = std::make_unique<WaylandDisplay>();
  display_->InitializeDisplay();
 
  gl_ = std::make_unique<GL>();
  gl_->init_egl(display_.get(), 0);

  return true;
}

void WaylandPlatform::createWindow(unsigned width, unsigned height,
    const char* vertShaderText, const char* fragShaderText,
    void (*drawPtr)(WaylandWindow*)) {
  struct sigaction sigint;

  display_->CreateAcceleratedSurface(width, height);
  gl_->init_gl(width, height, vertShaderText, fragShaderText);
  display_->GetWindow()->drawPtr = drawPtr;
  sigint.sa_handler = signal_int;
  sigemptyset(&sigint.sa_mask);
  sigint.sa_flags = SA_RESETHAND;
  sigaction(SIGINT, &sigint, NULL);
}

void WaylandPlatform::initGL() {
 

}

void WaylandPlatform::run() {
  display_->Run();
}

void WaylandPlatform::terminate() {
  gl_->finish_egl(display_.get());
  display_->Terminate();
}
