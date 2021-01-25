// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Copyright 2020 Joone Hur. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display.h"

#include <assert.h>
//#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <linux/input.h>

#include "wayland_platform.h"

const struct wl_registry_listener registry_listener = {
      WaylandDisplay::registry_handle_global, WaylandDisplay::registry_handle_global_remove};


static void pointer_handle_leave(void* data,
                                 struct wl_pointer* pointer,
                                 uint32_t serial,
                                 struct wl_surface* surface) {}

static void pointer_handle_motion(void* data,
                                  struct wl_pointer* pointer,
                                  uint32_t time,
                                  wl_fixed_t sx,
                                  wl_fixed_t sy) {}

static void pointer_handle_button(void* data,
                                  struct wl_pointer* wl_pointer,
                                  uint32_t serial,
                                  uint32_t time,
                                  uint32_t button,
                                  uint32_t state) {
  WaylandDisplay* display = static_cast<WaylandDisplay*>(data);
  WaylandWindow* window = display->GetWindow();

  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
    wl_shell_surface_move(window->shell_surface, display->seat,
                          serial);
}

static void pointer_handle_axis(void* data,
                                struct wl_pointer* wl_pointer,
                                uint32_t time,
                                uint32_t axis,
                                wl_fixed_t value) {}
static void pointer_handle_enter(void* data,
                                 struct wl_pointer* pointer,
                                 uint32_t serial,
                                 struct wl_surface* surface,
                                 wl_fixed_t sx,
                                 wl_fixed_t sy) {
  WaylandDisplay* display = static_cast<WaylandDisplay*>(data);
  struct wl_buffer* buffer;
  struct wl_cursor* cursor = display->default_cursor;
  struct wl_cursor_image* image;

  WaylandWindow* window = display->GetWindow();

  if (window->fullscreen)
    wl_pointer_set_cursor(pointer, serial, NULL, 0, 0);
  else if (cursor) {
    image = display->default_cursor->images[0];
    buffer = wl_cursor_image_get_buffer(image);
    wl_pointer_set_cursor(pointer, serial, display->cursor_surface,
                          image->hotspot_x, image->hotspot_y);
    wl_surface_attach(display->cursor_surface, buffer, 0, 0);
    wl_surface_damage(display->cursor_surface, 0, 0, image->width,
                      image->height);
    wl_surface_commit(display->cursor_surface);
  }
}

static const struct wl_pointer_listener pointer_listener = {
    pointer_handle_enter,  pointer_handle_leave, pointer_handle_motion,
    pointer_handle_button, pointer_handle_axis,
};

static void keyboard_handle_keymap(void* data,
                                   struct wl_keyboard* keyboard,
                                   uint32_t format,
                                   int fd,
                                   uint32_t size) {}

static void keyboard_handle_enter(void* data,
                                  struct wl_keyboard* keyboard,
                                  uint32_t serial,
                                  struct wl_surface* surface,
                                  struct wl_array* keys) {}

static void keyboard_handle_leave(void* data,
                                  struct wl_keyboard* keyboard,
                                  uint32_t serial,
                                  struct wl_surface* surface) {}

static void keyboard_handle_key(void* data,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t key,
                                uint32_t state) {
  WaylandDisplay* display = static_cast<WaylandDisplay*>(data);
  WaylandWindow* window = display->GetWindow();

  if (key == KEY_F11 && state)
    window->toggle_fullscreen();
  else if (key == KEY_ESC && state)
    running = 0;
}

static void keyboard_handle_modifiers(void* data,
                                      struct wl_keyboard* keyboard,
                                      uint32_t serial,
                                      uint32_t mods_depressed,
                                      uint32_t mods_latched,
                                      uint32_t mods_locked,
                                      uint32_t group) {}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap, keyboard_handle_enter,     keyboard_handle_leave,
    keyboard_handle_key,    keyboard_handle_modifiers,
};

static void seat_handle_capabilities(void* data,
                                     struct wl_seat* seat,
                                     uint32_t caps) {
  WaylandDisplay* d = static_cast<WaylandDisplay*>(data);

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !d->pointer) {
    d->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(d->pointer, &pointer_listener, d);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && d->pointer) {
    wl_pointer_destroy(d->pointer);
    d->pointer = NULL;
  }

  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !d->keyboard) {
    d->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(d->keyboard, &keyboard_listener, d);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && d->keyboard) {
    wl_keyboard_destroy(d->keyboard);
    d->keyboard = NULL;
  }
}

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
};


WaylandDisplay::WaylandDisplay() {

}

void WaylandDisplay::InitializeDisplay() {
  display_ = wl_display_connect(NULL);
  assert(display_);
 
  registry = wl_display_get_registry(display_);
  wl_registry_add_listener(registry, &registry_listener, this);
  wl_display_dispatch(display_);

  std::cout << "end of " << __func__ << std::endl;
}

void WaylandDisplay::CreateAcceleratedSurface(unsigned width, unsigned height) {
  cursor_surface = wl_compositor_create_surface(compositor);

  window_ = std::make_unique<WaylandWindow>();
  window_->display = this;
  window_->create_surface(width, height);
}

WaylandWindow* WaylandDisplay::GetWindow() {
  return window_.get();
}

void WaylandDisplay::Run() {
  int ret = 0;
  while (running && ret != -1)
    ret = wl_display_dispatch(display_);
}

void WaylandDisplay::Terminate() {
  window_->destroy_surface();

  wl_surface_destroy(cursor_surface);
  if (cursor_theme)
    wl_cursor_theme_destroy(cursor_theme);

  if (shell)
    wl_shell_destroy(shell);

  if (compositor)
    wl_compositor_destroy(compositor);

  wl_registry_destroy(registry);
  wl_display_flush(display_);
  wl_display_disconnect(display_);
}

void WaylandDisplay::registry_handle_global_remove(void* data,
                                   struct wl_registry* registry,
                                   uint32_t name) {}

void WaylandDisplay::registry_handle_global(void* data,
                            struct wl_registry* registry,
                            uint32_t name,
                            const char* interface,
                            uint32_t version) {
  std::cout << __func__ << std::endl;
  WaylandDisplay* d = static_cast<WaylandDisplay*>(data);

  if (strcmp(interface, "wl_compositor") == 0) {
    d->compositor =
        static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 1));
  } else if (strcmp(interface, "wl_shell") == 0) {
    d->shell = static_cast<wl_shell*>(wl_registry_bind(registry, name, &wl_shell_interface, 1));
  } else if (strcmp(interface, "wl_seat") == 0) {
    d->seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
    wl_seat_add_listener(d->seat, &seat_listener, d);
  } else if (strcmp(interface, "wl_shm") == 0) {
    d->shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
    d->cursor_theme = wl_cursor_theme_load(NULL, 32, d->shm);
    d->default_cursor = wl_cursor_theme_get_cursor(d->cursor_theme, "left_ptr");
  }
}
