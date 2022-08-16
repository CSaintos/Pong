#include <vulkan/vulkan.hpp>
//#define WINDOWS_BUILD
#ifdef WINDOWS_BUILD
#include <vulkan/vulkan_win32.h>
//#elif // Other OS
#endif
#include <iostream>

#include "vk_init.cpp"

#define stringify(name) #name

#define ArraySize(arr) sizeof((arr)) / sizeof((arr[0]))

#define VK_CHECK(result)                                  \
  if (result != VK_SUCCESS)                               \
  {                                                       \
    std::cout << "Vulkan Error: " << result << std::endl; \
    __debugbreak();                                       \
    return false;                                         \
  }

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
  VkDebugUtilsMessageTypeFlagBitsEXT msg_flags,
  const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
  void *p_user_data)
{
  std::cout << "Validation Error: " << p_callback_data->pMessage << std::endl;
  return false;
}

struct VkContext
{
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surface_format;
  VkPhysicalDevice gpu;
  VkDevice device;
  VkQueue graphics_queue;
  VkSwapchainKHR swapchain;
  VkCommandPool command_pool;

  VkSemaphore aquire_semaphore;
  VkSemaphore submit_semaphore;

  uint32_t sc_img_count;
  // TODO: Suballocation from Main Memory
  VkImage sc_images[5];

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
          VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
          VK_KHR_SURFACE_EXTENSION_NAME};
  
  char *layers[] =
  {
    "VK_LAYER_KHRONOS_validation"
  };

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.ppEnabledExtensionNames = extensions;
  instance_info.enabledExtensionCount = ArraySize(extensions);
  instance_info.ppEnabledLayerNames = layers;
  instance_info.enabledLayerCount = ArraySize(layers);

  VK_CHECK(vkCreateInstance(&instance_info, nullptr, &vk_context->instance));

  auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_context->instance, "vkCreateDebugUtilsMessengerEXT");

  if (vkCreateDebugUtilsMessengerEXT)
  {
    VkDebugUtilsMessengerCreateInfoEXT debug_info = {};
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_info.pfnUserCallback = vk_debug_callback;

    vkCreateDebugUtilsMessengerEXT(vk_context->instance, &debug_info, nullptr, &vk_context->debug_messenger);
  }
  else
  {
    return false;
  }

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
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = vk_context->graphics_idx;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    char *extensions[] = 
    {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.queueCreateInfoCount = 1;
    device_info.ppEnabledExtensionNames = extensions;
    device_info.queueCreateInfoCount = ArraySize(extensions);

    VK_CHECK(vkCreateDevice(vk_context->gpu, &device_info, nullptr, &vk_context->device));

    vkGetDeviceQueue(vk_context->device, vk_context->graphics_idx, 0, &vk_context->graphics_queue);
  }
  
  //> Swapchain
  {
    uint32_t format_count = 0;
    // TODO: Suballocation from Main Memory
    VkSurfaceFormatKHR surface_formats[10];
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vk_context->gpu, vk_context->surface, &format_count, nullptr));
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vk_context->gpu, vk_context->surface, &format_count, surface_formats));

    for (uint32_t i = 0; i < format_count; i++)
    {
      VkSurfaceFormatKHR format = surface_formats[i];

      if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
      {
        vk_context->surface_format = format;
        break;
      }
    }

    VkSurfaceCapabilitiesKHR surface_caps = {};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_context->gpu, vk_context->surface, &surface_caps));
    uint32_t img_count = 0;
    img_count = surface_caps.minImageCount + 1;
    img_count = img_count > surface_caps.maxImageCount ? img_count - 1 : img_count;

    VkSwapchainCreateInfoKHR sc_info = {};
    sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sc_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sc_info.surface = vk_context->surface;
    sc_info.imageFormat = vk_context->surface_format.format;
    sc_info.preTransform = surface_caps.currentTransform;
    sc_info.imageExtent = surface_caps.currentExtent;
    sc_info.minImageCount = img_count;
    sc_info.imageArrayLayers = 1;


    VK_CHECK(vkCreateSwapchainKHR(vk_context->device, &sc_info, nullptr, &vk_context->swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(vk_context->device, vk_context->swapchain, &vk_context->sc_img_count, nullptr));
    VK_CHECK(vkGetSwapchainImagesKHR(vk_context->device, vk_context->swapchain, &vk_context->sc_img_count, vk_context->sc_images));
  }

  //> Command Pool
  {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = vk_context->graphics_idx;

    VK_CHECK(vkCreateCommandPool(vk_context->device, &pool_info, nullptr, &vk_context->command_pool))
  }

  //> Sync Objects
  {
    VkSemaphoreCreateInfo sema_info = {};
    sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK(vkCreateSemaphore(vk_context->device, &sema_info, 0, &vk_context->aquire_semaphore));
    VK_CHECK(vkCreateSemaphore(vk_context->device, &sema_info, 0, &vk_context->submit_semaphore));
  }


  return true;
}

bool vk_render(VkContext *vk_context)
{
  uint32_t img_idx;
  VK_CHECK(vkAcquireNextImageKHR(vk_context->device, vk_context->swapchain, 0, vk_context->aquire_semaphore, 0, &img_idx));

  VkCommandBuffer cmd;
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = vk_context->command_pool;
  VK_CHECK(vkAllocateCommandBuffers(vk_context->device, &alloc_info, &cmd));

  VkCommandBufferBeginInfo begin_info = cmdBeginInfo();
  VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));

  //> Rendering Commands
  {
    VkClearColorValue color = {1, 1, 0, 1};
    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.layerCount = 1;
    range.levelCount = 1;

    vkCmdClearColorImage(cmd, vk_context->sc_images[img_idx], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &color, 1, &range);  
  }

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pWaitDstStageMask = &wait_stage;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;
  submit_info.pSignalSemaphores = &vk_context->submit_semaphore;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &vk_context->aquire_semaphore;
  submit_info.waitSemaphoreCount = 1;
  VK_CHECK(vkQueueSubmit(vk_context->graphics_queue, 1, &submit_info, 0));

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pSwapchains = &vk_context->swapchain;
  present_info.swapchainCount = 1;
  present_info.pImageIndices = &img_idx;
  present_info.pWaitSemaphores = &vk_context->submit_semaphore;
  present_info.waitSemaphoreCount = 1;
  VK_CHECK(vkQueuePresentKHR(vk_context->graphics_queue, &present_info));

  VK_CHECK(vkDeviceWaitIdle(vk_context->device));
  vkFreeCommandBuffers(vk_context->device, vk_context->command_pool, 1, &cmd);

  return true;
}

void vk_destroy(VkContext *vk_context)
{
  vkDestroyInstance(vk_context->instance, nullptr);
}