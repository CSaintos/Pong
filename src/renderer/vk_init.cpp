#include <vulkan/vulkan.hpp>

VkCommandBufferBeginInfo cmdBeginInfo()
{
  VkCommandBufferBeginInfo info;

  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  
  return info;
}