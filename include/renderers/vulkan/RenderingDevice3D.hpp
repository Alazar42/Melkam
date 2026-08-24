#pragma once

#include "components/Components3D.hpp"
#include "renderers/vulkan/VulkanContext.hpp"
#include <iostream>
#include <vector>

// Buffer resource allocated via VMA
struct GPUBuffer {
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  size_t size = 0;
};

// Godot 4-style Low-Level RenderingDevice for 3D Hardware Accelerated Pipelines
class RenderingDevice3D {
public:
  static RenderingDevice3D &get() {
    static RenderingDevice3D s_instance;
    return s_instance;
  }

  bool init(SDL_Window *window) {
    return VulkanContext::get().init(window);
  }

  void shutdown() {
    VulkanContext::get().shutdown();
  }

  // Creates a GPU Buffer (Vertex / Index / Uniform) with VMA
  GPUBuffer createBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
    GPUBuffer gpuBuf{};
    gpuBuf.size = size;

    if (!VulkanContext::get().initialized || size == 0) return gpuBuf;

    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memUsage;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo resultInfo{};
    if (vmaCreateBuffer(VulkanContext::get().allocator, &bufferInfo, &allocInfo,
                        &gpuBuf.buffer, &gpuBuf.allocation, &resultInfo) != VK_SUCCESS) {
      std::cerr << "[MelkamEngine::RenderingDevice3D] Failed to allocate GPU buffer of size: " << size << std::endl;
    }

    return gpuBuf;
  }

  void destroyBuffer(GPUBuffer &buf) {
    if (buf.buffer != VK_NULL_HANDLE && VulkanContext::get().allocator != VK_NULL_HANDLE) {
      vmaDestroyBuffer(VulkanContext::get().allocator, buf.buffer, buf.allocation);
      buf.buffer = VK_NULL_HANDLE;
      buf.allocation = VK_NULL_HANDLE;
    }
  }

  // Uploads raw vertex/index data into a GPU buffer
  void uploadData(const GPUBuffer &buf, const void *data, size_t size) {
    if (!VulkanContext::get().initialized || !buf.buffer || !data) return;
    void *mapped = nullptr;
    vmaMapMemory(VulkanContext::get().allocator, buf.allocation, &mapped);
    if (mapped) {
      std::memcpy(mapped, data, size);
      vmaUnmapMemory(VulkanContext::get().allocator, buf.allocation);
    }
  }
};
