#pragma once

#include "MainApp.h"
#include "Parser.h"
#include "Usage.h"
#include <random>


void WallpaperApplication::updateUniformBuffer(uint32_t currentImage)
{
    TotalFrames++;
    ubo.deltaTime = lastFrameTime * 2.0f;
    if (TotalFrames % 60 == 1)
    {
        SYSTEM_POWER_STATUS lpSystemPowerStatus;
        GetSystemPowerStatus(&lpSystemPowerStatus);
        lastCharge = lpSystemPowerStatus.BatteryLifePercent / 100.f;
    }
    ubo.charge = lastCharge;
    
    POINT p;
    GetCursorPos(&p);
    ubo.PosPrev2 = ubo.PosPrev;
    ubo.PosPrev = ubo.Pos;
    ubo.Pos = glm::vec2(p.x/1920.f, p.y/1080.f);
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
    ubo.Random = 1+rndDist(rndEngine);
    GetVolumeLevel(ubo.Volume);
    //auto currentTime = std::chrono::high_resolution_clock::now();
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

}


void WallpaperApplication::createShaderStorageBuffers() {
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
    std::random_device rd;   // non-deterministic generator
    std::mt19937 gen(rd());  // to seed mersenne twister.
    std::uniform_int_distribution<> dist(0001, 1000);
    std::uniform_int_distribution<> dist3(0, TextWidth*96-1);
    std::uniform_int_distribution<> dist10(0, 9);

    CoInitialize(0);

    // Initialize particles

    // Initial particle positions on a circle
    std::vector<Particle> particles(PARTICLE_COUNT);
    int i = 0;
    Parser Text;
    Text.init();
    for (auto& particle : particles) {
        i++;
        float r = 0.25f * sqrt(rndDist(rndEngine));
        float theta = rndDist(rndEngine) * 2.0f * 3.1415926f;
        float x = r * cos(theta);
        float y = r * sin(theta);
        particle.position = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
        particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.75f;
        float a = dist(gen) / 1000.f;
        particle.color = glm::vec4(0.2f, 0.9f, 1.f, 0.0f);
        particle.id = glm::vec2(a - 2, 0.f);

        if (i < 1024)
            particle.id.y = 1.0f;
        else if (i < 1536)
            particle.id.y = 2.0f;
        else if (i < 4096)
        {
            int guess = dist3(gen);
            while(Text.OutArray[guess] == 0)
                guess = dist3(gen);
            particle.color = glm::vec4( ( (guess % TextWidth) - 0.5f * TextWidth) / 1920.f * 1.2f, ((guess / TextWidth)/1080.f) * 1.2f + 0.66f,0.f,0.f);
            
            particle.id = glm::vec2(a -2.5f, 3.f);

        }
        else if (i < 5120)
        {
            particle.id.y = 4.0f;
            int pos = i%10;
            int sign = (pos >= 5 ? 1 : -1);
            pos = pos % 5;
            float offsetx[5] =  { 0.1f,0.2f,0.3f,0.4f,0.5f };
            float offsety[5] =  { 0.0f,-0.1f,0.1f,-0.2f,0.2f };
            float sizey[5] =    { 1.0f,1.1f,1.2f,1.3f,1.3f };
            particle.color = glm::vec4(sign * (offsetx[pos]*0.8f + 0.5f) , offsety[pos] - sizey[pos] * 0.5f - 0.05f, 0.03f, sizey[pos]);
        }
        else
        {
            particle.id.y = 5.0f;
        }
        /*
        if (particle.id.x && !Text.binit)
        {
            Text.init();
            int a[1280 * 96 / 32];
            for (int i = 0; i < 1280 * 96 / 32; i++)
            {
                a[i] = 0;
                for (int j = 0; j < 32; j++)
                    if (Text.OutArray[i * 32 + j] != 0)
                        a[i] |= 1 << j;
            }
            memcpy(staticUniformBufferMapped, a, sizeof(a));
        }
        */
    }

    VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

    // Create a staging buffer used to upload data to the gpu
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, particles.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    shaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    shaderStorageBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    // Copy initial particle data to all storage buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffers[i], shaderStorageBuffersMemory[i]);
        copyBuffer(stagingBuffer, shaderStorageBuffers[i], bufferSize);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void WallpaperApplication::drawFrame()
{

    Sleep(1000 / 40);
    // Compute submission        
    vkWaitForFences(device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    updateUniformBuffer(currentFrame);

    if (IsFullscreen())
    {
        return;
    }
    vkResetFences(device, 1, &computeInFlightFences[currentFrame]);

    vkResetCommandBuffer(computeCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    recordComputeCommandBuffer(computeCommandBuffers[currentFrame]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

    if (vkQueueSubmit(computeQueue, 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    };

    // Graphics submission
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}