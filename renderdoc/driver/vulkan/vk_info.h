/******************************************************************************
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Baldur Karlsson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include "vk_common.h"
#include "vk_manager.h"

#include "driver/shaders/spirv/spirv_common.h"

struct DescSetLayout
{
	void Init(const VkDescriptorSetLayoutCreateInfo* pCreateInfo);

	void CreateBindingsArray(vector<VkDescriptorInfo*> &descBindings);

	struct Binding
	{
		Binding() : immutableSampler(NULL) {}
		~Binding() { SAFE_DELETE_ARRAY(immutableSampler); }

		VkDescriptorType descriptorType;
		uint32_t arraySize;
		VkShaderStageFlags stageFlags;
		ResourceId *immutableSampler;
	};
	vector<Binding> bindings;

	uint32_t dynamicCount;
};
struct VulkanCreationInfo
{
	struct Pipeline
	{
		void Init(const VkGraphicsPipelineCreateInfo* pCreateInfo);
		void Init(const VkComputePipelineCreateInfo* pCreateInfo);

		ResourceId layout;
		ResourceId renderpass;
		uint32_t subpass;
		
		// VkGraphicsPipelineCreateInfo
		VkPipelineCreateFlags flags;

		// VkPipelineShaderStageCreateInfo
		ResourceId shaders[6];

		// VkPipelineVertexInputStateCreateInfo
		struct Binding
		{
			uint32_t vbufferBinding; // VKTODO I believe this is the meaning
			uint32_t bytestride;
			bool perInstance;
		};
		vector<Binding> vertexBindings;
		
		struct Attribute
		{
			uint32_t location;
			uint32_t binding;
			VkFormat format;
			uint32_t byteoffset;
		};
		vector<Attribute> vertexAttrs;

		// VkPipelineInputAssemblyStateCreateInfo
		VkPrimitiveTopology topology;
		bool primitiveRestartEnable;

		// VkPipelineTessellationStateCreateInfo
		uint32_t patchControlPoints;

		// VkPipelineViewportStateCreateInfo
		uint32_t viewportCount;
		vector<VkViewport> viewports;
		vector<VkRect2D> scissors;

		// VkPipelineRasterStateCreateInfo
		bool depthClipEnable;
		bool rasterizerDiscardEnable;
		VkFillMode fillMode;
		VkCullMode cullMode;
		VkFrontFace frontFace;
		bool depthBiasEnable;
		float depthBias;
		float depthBiasClamp;
		float slopeScaledDepthBias;
		float lineWidth;

		// VkPipelineMultisampleStateCreateInfo
		uint32_t rasterSamples;
		bool sampleShadingEnable;
		float minSampleShading;
		VkSampleMask sampleMask;

		// VkPipelineDepthStencilStateCreateInfo
		bool depthTestEnable;
		bool depthWriteEnable;
		VkCompareOp depthCompareOp;
		bool depthBoundsEnable;
		bool stencilTestEnable;
		VkStencilOpState front;
		VkStencilOpState back;
		float minDepthBounds;
		float maxDepthBounds;

		// VkPipelineColorBlendStateCreateInfo
		bool alphaToCoverageEnable;
		bool alphaToOneEnable;
		bool logicOpEnable;
		VkLogicOp logicOp;
		float blendConst[4];

		struct Attachment
		{
			bool blendEnable;

			struct BlendOp
			{
				VkBlend Source;
				VkBlend Destination;
				VkBlendOp Operation;
			} blend, alphaBlend;

			uint8_t channelWriteMask;
		};
		vector<Attachment> attachments;

		// VkPipelineDynamicStateCreateInfo
		bool dynamicStates[VK_DYNAMIC_STATE_NUM];
	};
	map<ResourceId, Pipeline> m_Pipeline;

	struct PipelineLayout
	{
		void Init(const VkPipelineLayoutCreateInfo* pCreateInfo);

		vector<ResourceId> descSetLayouts;
	};
	map<ResourceId, PipelineLayout> m_PipelineLayout;

	struct RenderPass
	{
		void Init(const VkRenderPassCreateInfo* pCreateInfo);

		vector<uint32_t> inputAttachments;
		vector<uint32_t> colorAttachments;
		int32_t depthstencilAttachment;

		uint32_t attachCount;

		VkRenderPass loadRP;
	};
	map<ResourceId, RenderPass> m_RenderPass;

	struct Framebuffer
	{
		void Init(const VkFramebufferCreateInfo* pCreateInfo);

		struct Attachment
		{
			ResourceId view;
		};
		vector<Attachment> attachments;
		
		uint32_t width, height, layers;
	};
	map<ResourceId, Framebuffer> m_Framebuffer;
	
	struct Buffer
	{
		void Init(const VkBufferCreateInfo* pCreateInfo);

		uint64_t size;
	};
	map<ResourceId, Buffer> m_Buffer;

	struct BufferView
	{
		void Init(const VkBufferViewCreateInfo* pCreateInfo);

		ResourceId buffer;
		uint64_t offset;
		uint64_t size;
	};
	map<ResourceId, BufferView> m_BufferView;
	
	struct Image
	{
		void Init(const VkImageCreateInfo* pCreateInfo);
	
		VkImageView view;

		VkImageType type;
		VkFormat format;
		VkExtent3D extent;
		int arraySize, mipLevels, samples;

		bool cube;
		uint32_t creationFlags;
	};
	map<ResourceId, Image> m_Image;

	struct ImageView
	{
		void Init(const VkImageViewCreateInfo* pCreateInfo);

		ResourceId image;
	};
	map<ResourceId, ImageView> m_ImageView;
	
	struct ShaderModule
	{
		void Init(const VkShaderModuleCreateInfo* pCreateInfo);

		SPVModule spirv;
		ShaderReflection reflTemplate;
		ShaderBindpointMapping mapping;
	};
	map<ResourceId, ShaderModule> m_ShaderModule;

	struct Shader
	{
		void Init(const VkShaderCreateInfo* pCreateInfo, VulkanCreationInfo::ShaderModule &moduleinfo);

		ResourceId module;
		ShaderReflection refl;
		ShaderBindpointMapping mapping;
	};
	map<ResourceId, Shader> m_Shader;
	
	map<ResourceId, string> m_Names;
	map<ResourceId, SwapchainInfo> m_SwapChain;
	map<ResourceId, DescSetLayout> m_DescSetLayout;
};
