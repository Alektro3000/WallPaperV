#pragma once

#include "MainApp.h"
#include "TextParser.h"
#include "Additional.h"
#include <random>
#include <numeric>
#include <cmath>

void WallpaperApplication::lazyUpdateUniform()
{        
    Additional::GetVolumeLevel(ubo.Volume);
    if (LazyUpdates % 5 != 0)
        return;

    SYSTEM_POWER_STATUS lpSystemPowerStatus;
    GetSystemPowerStatus(&lpSystemPowerStatus);
    if ((lpSystemPowerStatus.BatteryFlag & 128) == 128)
        ubo.charge = 0.9f;
    else
        ubo.charge = lpSystemPowerStatus.BatteryLifePercent / 100.f;
}

void WallpaperApplication::updateUniformBuffer(uint32_t currentImage)
{
    constexpr long long fact = 300000000; // once 0.3 seconds
    if (TotalTime / fact > LazyUpdates)
    {
        LazyUpdates = TotalTime / fact;
        lazyUpdateUniform();
    }


    constexpr long long fact1 = 70000000; // once 0.07 seconds
    if (TotalTime / fact1 > LazyUpdates1)
    {
        LazyUpdates1 = TotalTime / fact1;
        auto q = AudioCapture.GetSound(100);

        if (q.size() != 0)
            AudioSaved = q;
        else
            std::fill(AudioSaved.begin(), AudioSaved.end(), 0);
        
        AudioCapture.ClearRecord();
    }
    else
        AudioCapture.RecordSound();


    for (int i = 0; i < 100; i++)
    {
        double t = lastFrameTime * 40;
        if (t > 1)
            AudioSmoothed[i] = AudioSaved[i];
        else
            AudioSmoothed[i] = (1-t)*AudioSmoothed[i] + t*AudioSaved[i];
    }
    auto sum = std::reduce(AudioSmoothed.begin(), AudioSmoothed.end());
    auto min = *std::min_element(AudioSmoothed.begin(), AudioSmoothed.end());
    for (int i = 0; i < 25; i++)
    {
        ubo.Volumes[i].x = (float)(AudioSmoothed[i * 4 + 0] - min);
        ubo.Volumes[i].y = (float)(AudioSmoothed[i * 4 + 1] - min);
        ubo.Volumes[i].z = (float)(AudioSmoothed[i * 4 + 2] - min);
        ubo.Volumes[i].w = (float)(AudioSmoothed[i * 4 + 3] - min);
    }

    ubo.deltaTime = static_cast<float>(lastFrameTime * 2) * 0.5f;
    
    POINT p;
    GetCursorPos(&p);   
    RECT Pos = Additional::GetMonitorBoundingBox();

    ubo.PosPrev2 = ubo.PosPrev;
    ubo.PosPrev = ubo.Pos; 
    ubo.Pos = glm::vec2( (float)(p.x - Pos.left) / (Pos.right-Pos.left), (float)(p.y - Pos.top)/ (Pos.bottom-Pos.top));

    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
    ubo.Random = 1+rndDist(rndEngine);

    auto currentTime = std::chrono::high_resolution_clock::now();
    
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

}


void WallpaperApplication::createShaderStorageBuffers() {
    Additional::UpdateMonitors();
    ubo.MonitorCount = Additional::GetMonitorCount();
    {
        auto rect = Additional::GetMonitorBoundingBox();
        ubo.FullResolution = glm::vec2(rect.right-rect.left,rect.bottom-rect.top);
    }
    {
        if (ubo.MonitorCount <= 4)
        {
            auto bounding = Additional::GetMonitorBoundingBox();
            auto rects = Additional::GetMonitorsBoxes();
            std::transform(rects.begin(), rects.end(), ubo.Monitors,
                [res = ubo.FullResolution, bounding](RECT Inp)
                {return glm::vec4((Inp.left - bounding.left) / res.x*2-1, (Inp.top - bounding.top) / res.y * 2 - 1,
                    (Inp.right - Inp.left) / res.x*2, (Inp.bottom - Inp.top) / res.y * 2); });
        }
    }
    PARTICLE_COUNT_FACT = PARTICLE_COUNT * (ubo.MonitorCount-1) + PARTICLE_COUNT_Stable;
    PARTICLE_COUNT_SCALAR = ubo.MonitorCount;

    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
    std::random_device rd;   // non-deterministic generator
    std::mt19937 gen(rd());  // to seed mersenne twister.
    std::uniform_int_distribution<> dist(0001, 1000);
    std::uniform_int_distribution<> dist3(0, TextWidth*96-1);
    std::uniform_int_distribution<> dist10(0, 9);

    auto result = CoInitialize(0);
    // Initialize particles

    // Initial particle positions on a circle
    std::vector<Particle> particles(PARTICLE_COUNT_FACT);
    int i = 0;
    Parser Text;
    Text.init();

    for (auto& particle : particles) {
        i++;
        float r = 0.25f * sqrt(rndDist(rndEngine));
        float theta = rndDist(rndEngine) * 2.0f * 3.1415926f;
        float x = r * cos(theta);
        float y = r * sin(theta);
        particle.position = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), ((int) (rndDist(rndEngine)*1000)), rndDist(rndEngine));
        particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.75f;
        float a = dist(gen) / 1000.f;
        particle.color = glm::vec4(0.2f, 0.9f, 1.f, 0.0f);
        particle.id = glm::vec2(a - 2, 0.f);

        if (i < 1024 * PARTICLE_COUNT_SCALAR)
            particle.id.y = 1.0f;
        else if (i < 1536 * PARTICLE_COUNT_SCALAR)
            particle.id.y = 2.0f;
        else if (i < 6656 * PARTICLE_COUNT_SCALAR)
        {
            int guess = dist3(gen);
            while(Text.OutArray[guess] == 0 || (guess > TextWidth*96/2 && dist10(gen) < 6))
                guess = dist3(gen);
            float myfloat = particle.position.z;
            auto Monitor = ubo.Monitors[std::bit_cast<unsigned int>(myfloat)%ubo.MonitorCount];
            glm::vec2 Resolution = glm::vec2(Monitor.z,Monitor.w) * ubo.FullResolution/2.f;

            particle.color = glm::vec4(
                ((guess % TextWidth) - 0.5f * TextWidth) / Resolution.x * 1.2f,
                ((guess / TextWidth) / Resolution.y) * 1.2f + 0.75f, 0.f, 0.f);
            
            particle.id = glm::vec2(a - 2.5f, 3.f);
            particle.position.w = 0;
        }
        else if (i < 7680 * PARTICLE_COUNT_SCALAR)
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
    }

    VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT_FACT;

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

    // Compute submission        
    vkWaitForFences(device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
#ifdef NDEBUG 
    if (Additional::IsFullscreen())
    {
        Sleep(1000 / 30);
        return;
    }
#endif // NDEBUG 
    updateUniformBuffer(currentFrame);
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
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}