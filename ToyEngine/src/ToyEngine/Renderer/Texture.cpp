#include "Texture.hpp"

#include <vulkan/vulkan.hpp>

#include "Allocator.hpp"
#include "Buffer.hpp"
#include "GraphicsContext.hpp"
#include "ToyEngine/Renderer/Helpers.hpp"
#include "stb_image.h"
#include "tepch.hpp"

namespace TE {
void createTextureSampler(vk::Sampler& sampler) {
  vk::SamplerCreateInfo sampler_info{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = GraphicsContext::get().getDeviceProperties().limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0,
      .maxLod = 0,
      .borderColor = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = vk::False,
  };

  auto err = GraphicsContext::get().getDevice().createSampler(&sampler_info, nullptr, &sampler);
  if (err != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to create sampler");
  }
}

Texture::Texture(const std::string& path) {
  auto& ctx = GraphicsContext::get();

  int width, height, channels;
  stbi_uc* img = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  assert(img != nullptr);

  auto buffer = Buffer::createStagingBuffer(width * height * channels);
  buffer.write(img, width * height * channels, 0);
  stbi_image_free(img);

  vk::ImageCreateInfo image_info{
      .imageType = vk::ImageType::e2D,
      .format = vk::Format::eR8G8B8A8Srgb,
      .extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined,
  };

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

  auto err = vmaCreateImage(ctx.getAllocator(), (VkImageCreateInfo*)&image_info, &alloc_info,
                            &image, &allocation, nullptr);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("Failed to create image");
  }

  ctx.executeTransient([this, &buffer, width, height](vk::CommandBuffer cmd) {
    transistionImageLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    vk::BufferImageCopy region{
        .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
    };
    cmd.copyBufferToImage(buffer.getBuffer(), image, vk::ImageLayout::eTransferDstOptimal, 1,
                          &region);

    transistionImageLayout(cmd, vk::ImageLayout::eTransferDstOptimal,
                           vk::ImageLayout::eShaderReadOnlyOptimal);
  });

  img_view = createImageView(ctx.getDevice(), image, vk::Format::eR8G8B8A8Srgb);

  createTextureSampler(sampler);
}

Texture::~Texture() {
  auto& ctx = GraphicsContext::get();
  ctx.getDevice().destroySampler(sampler);
  ctx.getDevice().destroyImageView(img_view);
  vmaDestroyImage(ctx.getAllocator(), image, allocation);
}

void Texture::transistionImageLayout(vk::CommandBuffer cmd, vk::ImageLayout oldLayout,
                                     vk::ImageLayout newLayout) {
  vk::PipelineStageFlags src_stage;
  vk::PipelineStageFlags dst_stage;
  vk::ImageMemoryBarrier barrier{
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
  };

  if (oldLayout == vk::ImageLayout::eUndefined &&
      newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eNone;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    dst_stage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    src_stage = vk::PipelineStageFlagBits::eTransfer;
    dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::runtime_error("Unsupported layout transition!");
  }

  cmd.pipelineBarrier(src_stage, dst_stage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}
}  // namespace TE