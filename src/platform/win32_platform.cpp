#include <windows.h>

#include "renderer/vk_renderer.cpp"

static bool running = true;

LRESULT CALLBACK platform_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CLOSE:
    running = false;
    break;
  }

  return DefWindowProcA(window, msg, wParam, lParam);
}

bool platform_create_window(HWND window)
{
  HINSTANCE instance = GetModuleHandleA(0);

  WNDCLASSA wc = {};
  wc.lpfnWndProc = platform_window_callback;
  wc.hInstance = instance;
  wc.lpszClassName = "vulkan_engine_class";
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);

  if (!RegisterClassA(&wc))
  {
    MessageBoxA(window, "Failed registering window class", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }

  window = CreateWindowExA
  (
    WS_EX_APPWINDOW,
    "vulkan_engine_class",
    "Pong",
    WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPED,
    100, 100, 512, 512, 0, 0, instance, 0
  );

  if (window == 0)
  {
    MessageBoxA(window, "Failed creating window", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }

  ShowWindow(window, SW_SHOW);
  return true;
}

void platform_update_window(HWND window)
{
  MSG msg;

  while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

int main(int argc, char *argv[])
{
  VkContext vk_context = {};
  HWND window = 0;

  if (!platform_create_window(window))
  {
    return -1;
  }

  if (!vk_init(&vk_context))
  {
    return -1;
  }

  while (running)
  {
    platform_update_window(window);
  }

  vk_destroy(&vk_context);

  return 0;
}