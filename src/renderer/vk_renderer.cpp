#include <vulkan/vulkan.hpp>
//#define WINDOWS_BUILD
#ifdef WINDOWS_BUILD
#include <vulkan/vulkan_win32.h>
//#elif // Other OS
#endif
#include <iostream>

#define stringify(name) #name

#define ArraySize(arr) sizeof((arr)) / sizeof((arr[0]))

#define VK_CHECK(result)                                  \
  if (result != VK_SUCCESS)                               \
  {                                                       \
    std::cout << "Vulkan Error: " << result << std::endl; \
    __debugbreak();                                       \
    return false;                                         \
  }

struct VkContext
{
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice gpu;

  int graphics_idx;
};

bool vk_init(VkContext *vk_context, void *window)
{
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Pong";
  app_info.pEngineName = "Ponggine";

  char *extensions[] =
      {
#ifdef WINDOWS_BUILD
          VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
//#elif
#endif
          VK_KHR_SURFACE_EXTENSION_NAME};

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.ppEnabledExtensionNames = extensions;
  instance_info.enabledExtensionCount = ArraySize(extensions);

  VK_CHECK(vkCreateInstance(&instance_info, nullptr, &vk_context->instance));

  //> Create Surace
  {
#ifdef WINDOWS_BUILD
    VkWin32SurfaceCreateInfoKHR surface_info = {};
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.hwnd = (HWND)window;
    surface_info.hinstance = GetModuleHandleA(0);

    VK_CHECK(vkCreateWin32SurfaceKHR(vk_context->instance, &surface_info, nullptr, &vk_context->surface));
//#elif
#endif
  }

  //> Choose GPU
  {
    vk_context->graphics_idx = -1;
    uint32_t gpu_count = 0;
    // TODO: Suballocation from Main Allocation
    VkPhysicalDevice gpus[10];

    VK_CHECK(vkEnumeratePhysicalDevices(vk_context->instance, &gpu_count, nullptr));
    VK_CHECK(vkEnumeratePhysicalDevices(vk_context->instance, &gpu_count, gpus));

    for (uint32_t i = 0; i < gpu_count; i++)
    {
      VkPhysicalDevice gpu = gpus[i];

      uint32_t queue_family_count = 0;
      // TODO: Suballocation from Main Allocation
      VkQueueFamilyProperties queue_props[10];

      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_props);

      for (uint32_t j = 0; j < queue_family_count; j++)
      {
        if (queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          VkBool32 surface_support = VK_FALSE;
          VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, vk_context->surface, &surface_support));

          if (surface_support)
          {
            vk_context->graphics_idx = j;
            vk_context->gpu = gpu;
            break;
          }
        }
      }
    }

    if (vk_context->graphics_idx < 0)
    {
      return false;
    }
  }

  //> Logical Device
  {
    //vkCreateDevice(vk_context->gpu, &device_info, nullptr, &vk_context->device);
  }

  return true;
}

void vk_destroy(VkContext *vk_context)
{
  vkDestroyInstance(vk_context->instance, nullptr);
}