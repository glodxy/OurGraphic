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

  void SetDevice(VkDevice device, VmaAllocator allocator);

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
    dummy_image_view_ = image_view;
  }

 private:
  /**
   * 按位表示的COMMAND BUFFER，
   * 此处的MAX为10，所以有9个CommandBuffer， 默认有9位为1
   * */
  static constexpr uint32_t ALL_COMMAND_BUFFERS = (1 << MAX_COMMAND_BUFFERS_COUNT) - 1;

  struct PipelineKey {
    // 16 byte 所有的shader
    VkShaderModule shaders[SHADER_MODULE_COUNT];
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
    VkBuffer uniform_buffers[UBUFFER_BINGDING_COUNT]; // 绑定的uniform buffer
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

  using PipelineMap = std::unordered_map<PipelineKey, PipelineVal, PipelineHash, PipelineEqual>;
  using DescriptorMap = std::unordered_map<DescripotorKey, DescriptorBundle, DescHash, DescEqual>;

  /**
   * 该结构体描述了与CommandBuffer相关的状态
   * 1.pipeline的具体信息
   * 2.DescriptorSet
   * 3.裁剪窗口
   * */
  struct CmdBufferState {
    PipelineVal* current_pipeline = nullptr;
    DescriptorBundle* current_descriptor_bundle = nullptr;
    VkRect2D scissor = {};
  };

  /**
   * 获取或创建描述符
   * @param descriptors: 要创建或获得的描述符
   * @param bind: 如果为true，则代表需要调用vkCmdBindDescriptorSets
   * @return :代表有没有错
   * */
  bool GetOrCreateDescriptors(VkDescriptorSet descriptors[DESCRIPTOR_TYPE_COUNT],
                              bool* bind) noexcept;

  /**
   * @param bind:是否需要在外部进行绑定
   * 需要调用vkCmdBindPipeline
   * @return: 是否有错
   * */
  bool GetOrCreatePipeline(VkPipeline* pipeline, bool* bind) noexcept;

  void CreateLayoutsAndDescriptors() noexcept;
  void DestroyLayoutsAndDescriptors() noexcept;

  /**
   * 标识哪些command buffer的对应有修改
   * */
  void MarkDirtyPipeline() noexcept {
    //todo
    // 标识全修改
    dirty_pipeline_ = ALL_COMMAND_BUFFERS;
  }
  void MarkDirtyDescriptor() noexcept {
    //todo:
    dirty_descriptor_ = ALL_COMMAND_BUFFERS;
  }

 private:
  /**
   * 创建描述符池
   * @param size:池的大小
   * */
  VkDescriptorPool CreateDescriptorPool(uint32_t size) const;

  // 增长描述符池
  void GrowDescriptorPool() noexcept;



  VkDevice device_ = VK_NULL_HANDLE;
  VmaAllocator allocator_ = VK_NULL_HANDLE;
  // 默认的管线状态
  const RasterState default_raster_state_;

  // 当前的流水线
  PipelineKey current_pipeline_;
  // 当前的描述符
  DescripotorKey current_descriptor_;
  // 当前的cmd buffer状态 index
  uint32_t current_cmd_buffer_;

  /**
   * 该变量保存了所有command buffer对应的状态
   * */
  CmdBufferState cmd_buffer_state_[MAX_COMMAND_BUFFERS_COUNT];

  // todo：注释
  /**
   * 这两个变量都用来以bit的形式存储对应的脏位信息
   * 表示对应位的command buffer所属的pipeline或descriptor有修改
   * 有32位，
   * 为0代表有变化
   * */
  uint32_t dirty_pipeline_;
  uint32_t dirty_descriptor_;

  // 每个DescriptorSet对应的layout
  VkDescriptorSetLayout descriptor_set_layouts_[DESCRIPTOR_TYPE_COUNT] = {};

  // 所有的DescriptorSet
  /**
   * 相当于缓冲区，CommandBuffer变化时向里写入
   * */
  std::vector<VkDescriptorSet> descriptor_sets_[DESCRIPTOR_TYPE_COUNT];

  // 流水线的layout
  VkPipelineLayout pipeline_layout_;


  // 所有pipeline的cache
  PipelineMap pipelines_;
  // 所有Descriptor的cache
  DescriptorMap descriptors_;


  /**
   * Descriptor的池以及大小
   * */
   VkDescriptorPool descriptor_pool_;
   uint32_t descriptor_pool_size_ = 500;

   /**
    * 这两个变量用来临时存储需要销毁的Descriptor
    * 因为当DescriptorPool增长时需要重新分配，便需要把旧的删除
    * 这两个变量中的值会在若干帧后调用Destroy
    * */
    // 需要销毁的descriptor pool
    std::vector<VkDescriptorPool> extinct_descriptor_pools_;
    // 需要销毁的Descriptor
    std::vector<DescriptorBundle> extinct_descriptor_bundles_;

    /**
     * todo:补充注释
     * dummy的作用是对于那些不能使用VK_NULL_HANDLE表示的资源，需要提供一个空的默认实现来方便赋值
     * */
    VkImageView dummy_image_view_ = VK_NULL_HANDLE;
    VkDescriptorBufferInfo dummy_buffer_info_ = {};
    VkWriteDescriptorSet dummy_buffer_write_info_ = {};
    VkDescriptorImageInfo dummy_sampler_info_ = {};
    VkWriteDescriptorSet dummy_sampler_write_info_ = {};
    VkDescriptorImageInfo dummy_target_info_ = {};
    VkWriteDescriptorSet dummy_target_write_info_ = {};

    // todo:注释
    /**
     * 该dummy buffer用于清除那些未使用的Descriptor set 槽
     * */
    VkBuffer dummy_buffer_;
    VmaAllocation dummy_memory_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPIPELINECACHE_H_
