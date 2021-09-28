//
// Created by Glodxy on 2021/9/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPIPELINECACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPIPELINECACHE_H_
#include "../include_internal/DriverEnum.h"
#include "VulkanCommands.h"
#include "vk_mem_alloc.h"

#include <map>

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_HANDLE(VmaPool)

namespace our_graph {
class VulkanPipelineCache : public CommandBufferObserver {
 public:
  VulkanPipelineCache(VulkanPipelineCache const&) = delete;
  VulkanPipelineCache& operator=(VulkanPipelineCache const&) = delete;

  static constexpr uint32_t UBUFFER_BINGDING_COUNT = CONFIG_BINDING_COUNT;
  static constexpr uint32_t SAMPLER_BINDING_COUNT = MAX_SAMPLER_COUNT;
  static constexpr uint32_t TARGET_BINDING_COUNT = MAX_SUPPORTED_RENDER_TARGET_COUNT;
  static constexpr uint32_t SHADER_MODULE_COUNT = 2;
  static constexpr uint32_t VERTEX_ATTRIBUTE_COUNT = MAX_VERTEX_ATTRIBUTE_COUNT;


  static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 3;

  /**
   * 顶点缓存包括两部分：
   * 1.绑定的Buffer，代表了它具体的数据
   * 2.Attribute，代表了Shader如何取出该数据
   * */
  struct VertexArray {
    VkVertexInputAttributeDescription attribution[VERTEX_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription buffers[VERTEX_ATTRIBUTE_COUNT];
  };

  /**
   * 着色器程序包:
   * 1.顶点着色器
   * 2.片段着色器
   * */
  struct ProgramBundle {
    VkShaderModule vertex;
    VkShaderModule fragment;
  };

  struct RasterState {
    // 40bytes 光栅化的状态设置
    struct {
      VkBool32          depth_clamp_enable; // 是否启用深度约束
      VkBool32          rasterizer_discard_enable; // 是否启用光栅化裁剪
      VkPolygonMode     polygon_mode; // 多边形的模式
      VkCullModeFlags   cull_mode; // 裁剪模式
      VkFrontFace       front_face;  //前向面的定义
      VkBool32          depth_bias_enable;  // 是否启用深度偏移
      float             depth_bias_constant_factor;
      float             depth_bias_clamp;
      float             depth_bias_slope_factor;
      float             line_width; // 光栅化的线条宽带
    } rasterization;
    // 32 bytes RenderTarget的混合状态
    VkPipelineColorBlendAttachmentState blending;
    // 28 bytes 深度模板测试状态
    struct {
      VkBool32      depth_test_enable; // 是否启用深度测试
      VkBool32      depth_write_enable; // 是否启用深度写入
      VkCompareOp   depth_compare_op; // 深度测试的比较函数
      VkBool32      depth_bounds_test_enable; // 是否启用深度边界测试
      VkBool32      stencil_test_enable; // 是否启用模板测试
      float         min_depth_bounds; // 用于深度边界测试的下边界
      float         max_depth_bounds; // 用于深度边界测试的上边界
    } depth_stencil;
    // 20 bytes 多重采样状态
    struct {
      VkSampleCountFlagBits   rasterization_samples; // 光栅化的采样点
      VkBool32                sample_shading_enable; // 是否启用采样点着色
      float                   min_sample_shading; //
      VkBool32                alpha_to_coverage_enable; // alpha平均
      VkBool32                alpha_to_one_enable; // alpha取1剩值
    } multisampling;

    uint32_t color_target_count; // 有几个render target
  };

  // 该结构体描述了通用缓存的绑定
  struct UniformBufferBinding {
    VkBuffer buffer; //buffer
    VkDeviceSize offset; // 偏移量
    VkDeviceSize size; // 缓存大小
  };

  /**
   * 在该构造函数中不进行实际的Vulkan API调用，仅初始化相关的资源描述以及创建信息、
   * */
  VulkanPipelineCache();

  ~VulkanPipelineCache();

  void SetDevice(VkDevice device, VmaAllocator_T allocator);

  /**
   * 外部应该使用该函数来初始化一个RasterState的拷贝
   * */
  const RasterState& GetDefaultRasterSate() const {
    //todo: 返回默认状态
  }

  /**
   * 创建一个DescriptorSet并通过VkCmdBindDescriptorSets与Vulkan的实例绑定
   * */
  bool BindDescriptors(VkCommandBuffer cmd_buffer);

  /**
   * 创建一个Pipeline并通过VkCmdBindPipeline绑定
   * */
  void BindPipeline(VkCommandBuffer cmd_buffer) noexcept;

  /**
   * todo
   * 添加一个裁剪框
   * */
  void BindScissor(VkCommandBuffer cmd_buffer, VkRect2D scissor) noexcept;


  /**
   * 以下操作不涉及Vulkan
   * */
   // 设置着色器
  void BindProgramBundle(const ProgramBundle& bundle) noexcept;
  void BindRasterState(const RasterState& raster_state) noexcept;
  // 绑定RenderPass
  void BindRenderPass(VkRenderPass render_pass, int subpass_index) noexcept;
  // 设置几何类型
  void BindPrimitiveTopology(VkPrimitiveTopology topology) noexcept;
  void BindUniformBuffer(uint32_t binding_index, VkBuffer uniform_buffer,
                         VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) noexcept;
  // 绑定采样器
  void BindSamplers(VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT]) noexcept;
  // 绑定输入的纹理
  void BindInputAttachment(uint32_t binding_index, VkDescriptorImageInfo image_info) noexcept;
  // 绑定顶点缓存
  void BindVertexArray(const VertexArray& array) noexcept;


  /**
   * 获取指定位置的UBO
   * @param binding_index： 绑定的槽位
   * */
  UniformBufferBinding GetUniformBufferBinging(uint32_t binding_index) noexcept;

  /**
   * 1.检查该buffer是否有绑定到位置，如果有，将该位置设为 null
   * 2.禁用所有使用了该buffer的Descriptor
   * */
  void UnBindUniformBuffer(VkBuffer uniform_buffer) noexcept;

  /**
   * 1.检查是否有绑定的采样器，如果有则reset
   * 2.禁用相关的Descriptor
   * */
  void UnBindImageView(VkImageView image_view) noexcept;

  /**
   * 该函数应该在切换VkDevice之前完成，
   * 销毁所有Vulkan对象
   * */
  void DestroyAllCache() noexcept;

  /**
   *因为Pipeline和DescriptorSet都是和CommandBuffer绑定的
   * 所以在CommandBuffer改变时需要响应变化
   * */
  void OnCommandBuffer(const VulkanCommandBuffer& cmd_buffer) override;

  /**
   *设置虚拟纹理来清除老的DescriptorSet
   * */
  void SetDummyTexture(VkImageView image_view) {
    //todo
  }

 private:
  /**
   * 按位表示的COMMAND BUFFER，
   * 此处的MAX为10，所以有9个CommandBuffer， 默认有9位为1
   * */
  static constexpr uint32_t ALL_COMMAND_BUFFERS = (1 << MAX_COMMAND_BUFFERS_COUNT) - 1;

  struct PipelineKey {
    // 16 byte 所有的shader
    VkShaderModule shaders{SHADER_MODULE_COUNT};
    // 124 byte 管线状态
    RasterState raster_state;
    // 4 bytes 几何类型
    VkPrimitiveTopology topology;
    // 8 byte 管线的renderpass
    VkRenderPass render_pass;
    // 2 byte
    uint16_t subpass_index;
    uint16_t padding0; // 2 byte
    // 256 byte 绑定的Attribute
    VkVertexInputAttributeDescription vertex_attributes[VERTEX_ATTRIBUTE_COUNT];
    // 192 byte 绑定的输入顶点缓存
    VkVertexInputBindingDescription vertex_buffer[VERTEX_ATTRIBUTE_COUNT];
    uint32_t padding1; // 4 byte
  };

  struct PipelineHash {
    std::size_t operator()(const PipelineKey& key) const;
  };

  struct PipelineEqual {
    bool operator()(const PipelineKey& k1, const PipelineKey& k2) const;
  };

  /**
   * age: 该数字代表了从上次的使用到现在经过了多少次CommandBuffer的提交刷新
   * 用于LRU缓存策略.
   * */
  struct PipelineVal {
    VkPipeline handle;
    uint32_t age;
  };

  #pragma pack(push, 1)
  struct DescripotorKey {
    VkBuffer uniform_buffers[UBUFFER_BINGDING_COUNT];
    VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT];
    VkDescriptorImageInfo input_attachments[TARGET_BINDING_COUNT];
    VkDeviceSize uniform_buffer_offsets[UBUFFER_BINGDING_COUNT];
    VkDeviceSize uniform_buffer_sizes[UBUFFER_BINGDING_COUNT];
  };
  #pragma pack(pop)

  struct DescHash {
    std::size_t operator()(const DescripotorKey& key) const;
  };

  struct DescEqual {
    bool operator()(const DescripotorKey& k1, const DescripotorKey& k2) const;
  };

  struct DescriptorBundle {
    VkDescriptorSet handles[DESCRIPTOR_TYPE_COUNT];
    /**
     * 32 bit， 按位表示的command buffer使用
     * */
    uint32_t command_buffers;
  };

  using PipelineMap = std::map<PipelineKey, PipelineVal, PipelineHash, PipelineEqual>;
  using DescriptorMap = std::map<DescripotorKey, DescriptorBundle, DescHash, DescEqual>;

  struct CmdBufferState {
    PipelineVal* current_pipeline = nullptr;
    DescriptorBundle* current_descriptor_bundle = nullptr;
    VkRect2D scissor = {};
  };
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPIPELINECACHE_H_
