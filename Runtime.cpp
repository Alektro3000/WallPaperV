
#include "app.hpp"


void WallpaperApplication::updateUniformBuffer(uint32_t currentImage)
{
static auto startTime = std::chrono::high_resolution_clock::now();

auto currentTime = std::chrono::high_resolution_clock::now();
float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

UniformBufferObject ubo{};
//ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
//ubo.proj[1][1] *= -1;
ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 0.0f));
ubo.view = glm::scale(ubo.view, glm::vec3(2.0f / swapChainExtent.width, 2.0f / swapChainExtent.height, 1.0f));
memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
   }