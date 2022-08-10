#include <vulkan/vulkan.hpp>

struct VkContext
{
  VkInstance instance;
};

bool vk_init(VkContext *vk_context) 
{
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Pong";
  app_info.pEngineName = "Ponggine";

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  VkResult res = vkCreateInstance(&instance_info, nullptr, &vk_context->instance);

  if (res == VK_SUCCESS)
  {
    return true;
  }

  return false;
}

void vk_destroy(VkContext *vk_context)
{
  vkDestroyInstance(vk_context->instance, nullptr);
}