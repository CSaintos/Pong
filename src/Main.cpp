#include <iostream>
#include <vulkan/vulkan.hpp>

int main(int argc, char *argv[])
{
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Pong";
  app_info.pEngineName = "Ponggine";

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  VkInstance instance;

  VkResult res = vkCreateInstance(&instance_info, nullptr, &instance);

  if (res == VK_SUCCESS)
  {
    std::cout << "Successfully created vulkan instance" << std::endl;
  }
  else
  {
    std::cout << "Failed to create vulkan instance" << std::endl;
  }

  vkDestroyInstance(instance, nullptr);
  std::cout << "Successfully destroyed vulkan instance" << std::endl;

  return 0;
}