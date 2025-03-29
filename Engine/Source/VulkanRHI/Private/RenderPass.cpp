
#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	RenderPass::RenderPass(VulkanDevice* device, const RenderPass::Descriptor& descriptor)
		: device(device)
		, desc(descriptor)
	{
		VkRenderPassCreateInfo renderPassCI{};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.flags = 0;
		
		FixedArray<VkAttachmentDescription, RHI::Limits::Pipeline::MaxRenderAttachmentCount> attachments{};
		
		for (const RenderPass::AttachmentBinding& attachmentBinding : desc.attachments)
		{
			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.initialLayout = attachmentBinding.initialLayout;
			attachmentDesc.finalLayout = attachmentBinding.finalLayout;
			attachmentDesc.format = RHIFormatToVkFormat(attachmentBinding.format);
			attachmentDesc.loadOp = RHIAttachmentLoadActionToVk(attachmentBinding.loadStoreAction.loadAction);
			attachmentDesc.storeOp = RHIAttachmentStoreActionToVk(attachmentBinding.loadStoreAction.storeAction);
			attachmentDesc.stencilLoadOp = RHIAttachmentLoadActionToVk(attachmentBinding.loadStoreAction.loadActionStencil);
			attachmentDesc.stencilStoreOp = RHIAttachmentStoreActionToVk(attachmentBinding.loadStoreAction.storeActionStencil);
			attachmentDesc.samples = GetSampleCountFlagBits(attachmentBinding.multisampleState.sampleCount);
			
			attachments.Add(attachmentDesc);
		}
		
		renderPassCI.attachmentCount = attachments.GetSize();
		renderPassCI.pAttachments = attachments.GetData();

		FixedArray<VkSubpassDescription, RHI::Limits::Pipeline::MaxSubPassCount> subpasses{};

		StaticArray<FixedArray<VkAttachmentReference, RHI::Limits::Pipeline::MaxColorAttachmentCount>, 
			RHI::Limits::Pipeline::MaxSubPassCount> colorAttachments{};
		StaticArray<FixedArray<VkAttachmentReference, RHI::Limits::Pipeline::MaxColorAttachmentCount>,
			RHI::Limits::Pipeline::MaxSubPassCount> resolveAttachments{};
		StaticArray<FixedArray<VkAttachmentReference, RHI::Limits::Pipeline::MaxColorAttachmentCount>,
			RHI::Limits::Pipeline::MaxSubPassCount> subpassInputAttachments{};
		StaticArray<FixedArray<u32, RHI::Limits::Pipeline::MaxColorAttachmentCount>,
			RHI::Limits::Pipeline::MaxSubPassCount> preserveAttachments{};
		StaticArray<FixedArray<VkAttachmentReference, RHI::Limits::Pipeline::MaxColorAttachmentCount>,
			RHI::Limits::Pipeline::MaxSubPassCount> depthStencilAttachments{};
		int i = 0;

		for (const RenderPass::SubPassDescriptor& subpassDesc : desc.subpasses)
		{
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			
			for (const RenderPass::SubPassAttachment& subpassAttachment : subpassDesc.colorAttachments)
			{
				VkAttachmentReference ref{};
				ref.attachment = subpassAttachment.attachmentIndex;
				ref.layout = subpassAttachment.imageLayout;
				colorAttachments[i].Add(ref);
			}

			subpass.colorAttachmentCount = colorAttachments[i].GetSize();
			subpass.pColorAttachments = colorAttachments[i].GetData();

			for (const RenderPass::SubPassAttachment& subpassAttachment : subpassDesc.resolveAttachments)
			{
				VkAttachmentReference ref{};
				ref.attachment = subpassAttachment.attachmentIndex;
				ref.layout = subpassAttachment.imageLayout;
				resolveAttachments[i].Add(ref);
			}

			if (!resolveAttachments[i].IsEmpty())
				subpass.pResolveAttachments = resolveAttachments[i].GetData();
			
			for (const RenderPass::SubPassAttachment& subpassAttachment : subpassDesc.subpassInputAttachments)
			{
				VkAttachmentReference ref{};
				ref.attachment = subpassAttachment.attachmentIndex;
				ref.layout = subpassAttachment.imageLayout;
				subpassInputAttachments[i].Add(ref);
			}

			subpass.inputAttachmentCount = subpassInputAttachments[i].GetSize();
			subpass.pInputAttachments = subpassInputAttachments[i].GetData();
			
			for (u32 index : subpassDesc.preserveAttachments)
			{
				preserveAttachments[i].Add(index);
			}

			subpass.preserveAttachmentCount = preserveAttachments[i].GetSize();
			subpass.pPreserveAttachments = preserveAttachments[i].GetData();

			subpass.pDepthStencilAttachment = nullptr;
			if (subpassDesc.depthStencilAttachment.GetSize() > 0)
			{
				VkAttachmentReference ref{};
				ref.attachment = subpassDesc.depthStencilAttachment[0].attachmentIndex;
				ref.layout = subpassDesc.depthStencilAttachment[0].imageLayout;
				depthStencilAttachments[i].Add(ref);

				subpass.pDepthStencilAttachment = &depthStencilAttachments[i].GetLast();
			}

			subpasses.Add(subpass);

			i++;
		}

		renderPassCI.subpassCount = subpasses.GetSize();
		renderPassCI.pSubpasses = subpasses.GetData();

		renderPassCI.dependencyCount = desc.dependencies.GetSize();
		if (desc.dependencies.NotEmpty())
			renderPassCI.pDependencies = desc.dependencies.GetData();

		auto result = vkCreateRenderPass(device->GetHandle(), &renderPassCI, VULKAN_CPU_ALLOCATOR, &renderPass);
		if (result != VK_SUCCESS)
		{
			return;
		}

		String name = descriptor.name.GetString();

#if PLATFORM_DESKTOP && !CE_BUILD_RELEASE
		device->SetObjectDebugName((uint64_t)renderPass, VK_OBJECT_TYPE_RENDER_PASS, name.GetCString());
#endif
	}

	RenderPass::~RenderPass()
	{
		if (renderPass != nullptr)
		{
			vkDestroyRenderPass(device->GetHandle(), renderPass, VULKAN_CPU_ALLOCATOR);
			renderPass = nullptr;
		}
	}

	void RenderPass::BuildDescriptor(Vulkan::Scope* pass, RenderPass::Descriptor& outDescriptor)
	{
		outDescriptor = {};
		if (pass == nullptr)
			return;
		
		if (pass->nextSubPass != nullptr && pass->prevSubPass == nullptr) // Multiple subpasses
		{
			Scope* cur = pass;
			outDescriptor.attachments.Clear();
			outDescriptor.dependencies.Clear();
			outDescriptor.subpasses.Clear();
			HashMap<RHI::AttachmentID, RHI::ImageFrameAttachment*> attachmentsById{};
			HashMap<RHI::AttachmentID, AttachmentBinding> attachmentBindingsById{};
			HashMap<RHI::AttachmentID, int> attachmentIndicesById{};
			Array<RHI::AttachmentID> attachmentIdsInOrder{};
			int totalAttachmentCount = 0;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			outDescriptor.dependencies.Add(dependency);
			int subpassIndex = 0;

			while (cur != nullptr)
			{
				Scope* next = (Vulkan::Scope*)cur->nextSubPass;

				SubPassDescriptor subPassDesc{};
				HashSet<int> usedAttachmentIndices{};

				for (auto scopeAttachment : cur->attachments)
				{
					if (!scopeAttachment->IsImageAttachment() || scopeAttachment->GetFrameAttachment() == nullptr ||
						!scopeAttachment->GetFrameAttachment()->IsImageAttachment())
						continue;
					if (scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Shader ||
						scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Copy)
						continue;

					RHI::ImageScopeAttachment* imageScopeAttachment = (RHI::ImageScopeAttachment*)scopeAttachment;
					RHI::ImageFrameAttachment* imageFrameAttachment = (RHI::ImageFrameAttachment*)scopeAttachment->GetFrameAttachment();
					auto attachmentId = imageFrameAttachment->GetId();

					RHI::Format format;

					if (imageFrameAttachment->GetLifetimeType() == RHI::AttachmentLifetimeType::Transient)
					{
						format = imageFrameAttachment->GetImageDescriptor().format;
					}
					else
					{
						RHI::RHIResource* resource = imageFrameAttachment->GetResource();
						if (resource == nullptr)
							continue;
						if (resource->GetResourceType() == RHI::ResourceType::Texture)
						{
							Texture* image = (Texture*)resource;
							format = image->GetFormat();
						}
						else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
						{
							TextureView* imageView = (TextureView*)resource;
							format = imageView->GetFormat();
						}
						else
						{
							continue;
						}
					}

					bool initialUse = false;

					if (!attachmentsById.KeyExists(attachmentId))
					{
						initialUse = true;
						attachmentsById[attachmentId] = imageFrameAttachment;

						AttachmentBinding attachmentBinding{};
						attachmentBinding.attachmentId = attachmentId;
						attachmentBinding.format = format;
						attachmentBinding.loadStoreAction = imageScopeAttachment->GetLoadStoreAction();
						attachmentBinding.multisampleState = imageScopeAttachment->GetMultisampleState();

						attachmentBindingsById[attachmentId] = attachmentBinding;
						attachmentIndicesById[attachmentId] = totalAttachmentCount++;
						attachmentIdsInOrder.Add(attachmentId);
					}

					SubPassAttachment attachmentRef{};
					
					auto loadStoreAction = imageScopeAttachment->GetLoadStoreAction();
					attachmentBindingsById[attachmentId].loadStoreAction.storeAction = loadStoreAction.storeAction;
					attachmentBindingsById[attachmentId].loadStoreAction.storeActionStencil = loadStoreAction.storeActionStencil;

					usedAttachmentIndices.Add(attachmentIndicesById[attachmentId]);
					
					switch (scopeAttachment->GetUsage())
					{
					case RHI::ScopeAttachmentUsage::DepthStencil:
						attachmentRef.attachmentIndex = attachmentIndicesById[attachmentId];
						if (EnumHasFlag(scopeAttachment->GetAccess(), RHI::ScopeAttachmentAccess::Write))
						//if (true)
						{
							attachmentRef.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
							if (initialUse)
								attachmentBindingsById[attachmentId].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
							attachmentBindingsById[attachmentId].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						}
						else
						{
							attachmentRef.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
							if (initialUse)
								attachmentBindingsById[attachmentId].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
							attachmentBindingsById[attachmentId].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						}
						subPassDesc.depthStencilAttachment.Add(attachmentRef);
						break;
					case RHI::ScopeAttachmentUsage::Color:
						attachmentRef.attachmentIndex = attachmentIndicesById[attachmentId];
						attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						if (initialUse)
							attachmentBindingsById[attachmentId].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						attachmentBindingsById[attachmentId].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						subPassDesc.colorAttachments.Add(attachmentRef);
						break;
					case RHI::ScopeAttachmentUsage::SubpassInput:
						attachmentRef.attachmentIndex = attachmentIndicesById[attachmentId];
						attachmentRef.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						if (initialUse)
							attachmentBindingsById[attachmentId].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						attachmentBindingsById[attachmentId].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						subPassDesc.subpassInputAttachments.Add(attachmentRef);
						break;
					case RHI::ScopeAttachmentUsage::Resolve:
						attachmentRef.attachmentIndex = attachmentIndicesById[attachmentId];
						attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						if (initialUse)
							attachmentBindingsById[attachmentId].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						attachmentBindingsById[attachmentId].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						subPassDesc.resolveAttachments.Add(attachmentRef);
						break;
					}
				}

				for (auto [attachmentId, attachmentIndex] : attachmentIndicesById)
				{
					if (!usedAttachmentIndices.Exists(attachmentIndex))
					{
						subPassDesc.preserveAttachments.Add(attachmentIndex);
					}
				}

				Scope* prev = (Vulkan::Scope*)cur->prevSubPass;

				while (prev != nullptr) // Compile Subpass dependencies with ALL of previous passes
				{
					HashMap<ScopeAttachment*, ScopeAttachment*> commonAttachments{};
					if (prev != nullptr)
						commonAttachments = Scope::FindCommonFrameAttachments(prev, cur);

					for (auto [from, to] : commonAttachments)
					{
						auto fromUsage = from->GetUsage();
						auto fromAccess = from->GetAccess();
						auto toUsage = to->GetUsage();
						auto toAccess = to->GetAccess();

						// No dependency if both usages are Read-Only
						if (!EnumHasFlag(fromAccess, RHI::ScopeAttachmentAccess::Write) &&
							!EnumHasFlag(toAccess, RHI::ScopeAttachmentAccess::Write))
							continue;

						VkSubpassDependency dependency{};
						dependency.srcSubpass = prev->subpassIndex;//subpassIndex - 1;
						dependency.dstSubpass = cur->subpassIndex;//subpassIndex;
						dependency.dependencyFlags = 0;

						bool isRegional = true;

						switch (fromUsage)
						{
						case RHI::ScopeAttachmentUsage::Color:
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							break;
						case RHI::ScopeAttachmentUsage::DepthStencil:
							dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							if (EnumHasFlag(fromAccess, RHI::ScopeAttachmentAccess::Write))
							{
								dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							}
							break;
						case RHI::ScopeAttachmentUsage::SubpassInput:
							isRegional = false;
							dependency.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
							dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
							break;
						case RHI::ScopeAttachmentUsage::Resolve:
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							break;
						case RHI::ScopeAttachmentUsage::Shader:
							isRegional = false;
							dependency.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
							dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
							if (EnumHasFlag(fromAccess, RHI::ScopeAttachmentAccess::Write))
							{
								dependency.srcAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
							}
							break;
						default:
							continue;
						}

						switch (toUsage)
						{
						case RHI::ScopeAttachmentUsage::Color:
							dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							break;
						case RHI::ScopeAttachmentUsage::DepthStencil:
							dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							if (EnumHasFlag(toAccess, RHI::ScopeAttachmentAccess::Write))
							{
								dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							}
							break;
						case RHI::ScopeAttachmentUsage::SubpassInput:
							isRegional = false;
							dependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
							dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
							break;
						case RHI::ScopeAttachmentUsage::Resolve:
							dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							break;
						case RHI::ScopeAttachmentUsage::Shader:
							isRegional = false;
							dependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
							dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
							if (EnumHasFlag(toAccess, RHI::ScopeAttachmentAccess::Write))
							{
								dependency.dstAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
							}
							break;
						default:
							continue;
						}

						if (isRegional)
						{
							dependency.dependencyFlags |= VK_DEPENDENCY_BY_REGION_BIT;
						}

						outDescriptor.dependencies.Add(dependency);
					}

					prev = (Vulkan::Scope*)prev->prevSubPass;
				}

				outDescriptor.subpasses.Add(subPassDesc);

				subpassIndex++;
				cur = next;
			}

			for (const auto& attachmentId : attachmentIdsInOrder)
			{
				outDescriptor.attachments.Add(attachmentBindingsById[attachmentId]);
			}
		}
		else // Single subpass
		{
			outDescriptor.attachments.Clear();
			HashMap<AttachmentID, AttachmentBinding*> attachmentBindingsById{};

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			outDescriptor.dependencies.Add(dependency);

			SubPassDescriptor subpass{};
			int i = 0;

			for (auto scopeAttachment : pass->attachments)
			{
				if (!scopeAttachment->IsImageAttachment() || scopeAttachment->GetFrameAttachment() == nullptr ||
					!scopeAttachment->GetFrameAttachment()->IsImageAttachment())
					continue;
				if (scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Shader ||
					scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Copy)
					continue;
                
				RHI::ImageScopeAttachment* imageScopeAttachment = (RHI::ImageScopeAttachment*)scopeAttachment;
				ImageFrameAttachment* imageFrameAttachment = (ImageFrameAttachment*)scopeAttachment->GetFrameAttachment();
				RHI::Format format;
				u32 sampleCount = 1;

				if (imageFrameAttachment->GetLifetimeType() == RHI::AttachmentLifetimeType::Transient)
				{
					format = imageFrameAttachment->GetImageDescriptor().format;
					sampleCount = imageFrameAttachment->GetImageDescriptor().sampleCount;
				}
				else
				{
					RHIResource* resource = imageFrameAttachment->GetResource();
					if (resource == nullptr)
						continue;

					if (resource->GetResourceType() == RHI::ResourceType::Texture)
					{
						Texture* image = (Texture*)resource;
						format = image->GetFormat();
						sampleCount = image->GetSampleCount();
					}
					else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
					{
						TextureView* imageView = (TextureView*)resource;
						if (imageView->GetTexture() == nullptr)
							continue;
						format = imageView->GetFormat();
						sampleCount = imageView->GetTexture()->GetSampleCount();
					}
					else
					{
						continue;
					}
				}

				AttachmentBinding attachmentBinding{};
				attachmentBinding.format = format;
				attachmentBinding.loadStoreAction = scopeAttachment->GetLoadStoreAction();
				attachmentBinding.multisampleState = scopeAttachment->GetMultisampleState();
				attachmentBinding.attachmentId = scopeAttachment->GetId();
				attachmentBinding.multisampleState.sampleCount = sampleCount;

				SubPassAttachment attachmentRef{};

				switch (scopeAttachment->GetUsage())
				{
				case RHI::ScopeAttachmentUsage::DepthStencil:
					attachmentRef.attachmentIndex = i++;
					if (EnumHasFlag(scopeAttachment->GetAccess(), RHI::ScopeAttachmentAccess::Write))
					//if (true)
					{
						attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						attachmentRef.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					}
					else
					{
						attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						attachmentRef.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					}
					subpass.depthStencilAttachment.Add(attachmentRef);
					break;
				case RHI::ScopeAttachmentUsage::Color:
					attachmentRef.attachmentIndex = i++;
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					subpass.colorAttachments.Add(attachmentRef);
					break;
				case RHI::ScopeAttachmentUsage::SubpassInput:
					attachmentRef.attachmentIndex = i++;
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					subpass.subpassInputAttachments.Add(attachmentRef);
					break;
				case RHI::ScopeAttachmentUsage::Resolve:
					attachmentRef.attachmentIndex = i++;
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					subpass.resolveAttachments.Add(attachmentRef);
					break;
				}
				
				outDescriptor.attachments.Add(attachmentBinding);
				attachmentBindingsById[scopeAttachment->GetId()] = &outDescriptor.attachments.GetLast();
			}

			outDescriptor.subpasses.Add(subpass);
		}
	}

	void RenderPass::BuildDescriptor(const RHI::RenderTargetLayout& rtLayout, Descriptor& outDescriptor)
	{
		outDescriptor = {};

		for (int i = 0; i < rtLayout.attachmentLayouts.GetSize(); i++)
		{
			const auto& attachmentLayout = rtLayout.attachmentLayouts[i];
			if (attachmentLayout.attachmentUsage == RHI::ScopeAttachmentUsage::Shader ||
				attachmentLayout.attachmentUsage == RHI::ScopeAttachmentUsage::Copy)
				continue;

			AttachmentBinding attachmentBinding{};
			attachmentBinding.attachmentId = attachmentLayout.attachmentId;
			attachmentBinding.format = attachmentLayout.format;
			attachmentBinding.multisampleState = attachmentLayout.multisampleState;
			attachmentBinding.loadStoreAction.loadAction = attachmentLayout.loadAction;
			attachmentBinding.loadStoreAction.storeAction = attachmentLayout.storeAction;
			attachmentBinding.loadStoreAction.loadActionStencil = attachmentLayout.loadActionStencil;
			attachmentBinding.loadStoreAction.storeActionStencil = attachmentLayout.storeActionStencil;

			switch (attachmentLayout.attachmentUsage)
			{
			case RHI::ScopeAttachmentUsage::Color:
				attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case RHI::ScopeAttachmentUsage::DepthStencil:
				attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case RHI::ScopeAttachmentUsage::Resolve:
				attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case RHI::ScopeAttachmentUsage::SubpassInput:
				attachmentBinding.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				attachmentBinding.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				break;
			}

			outDescriptor.attachments.Add(attachmentBinding);
		}

		HashSet<u32> renderPassUsedAttachments{};

		if (rtLayout.subpasses.GetSize() == 0) // No subpasses provided by user, automatically create one based on all attachments
		{
			SubPassDescriptor subpass{};

			for (int i = 0; i < rtLayout.attachmentLayouts.GetSize(); i++)
			{
				SubPassAttachment attachmentRef{};

				switch (rtLayout.attachmentLayouts[i].attachmentUsage)
				{
				case RHI::ScopeAttachmentUsage::Color:
					attachmentRef.attachmentIndex = i;
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					subpass.colorAttachments.Add(attachmentRef);
					break;
				case RHI::ScopeAttachmentUsage::DepthStencil:
					attachmentRef.attachmentIndex = i;
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					subpass.depthStencilAttachment.Add(attachmentRef);
					break;
				case RHI::ScopeAttachmentUsage::Resolve:
					attachmentRef.attachmentIndex = i;
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					subpass.resolveAttachments.Add(attachmentRef);
					break;
				case RHI::ScopeAttachmentUsage::SubpassInput:
					attachmentRef.attachmentIndex = i;
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					subpass.resolveAttachments.Add(attachmentRef);
					break;
				}
			}

			outDescriptor.subpasses.Add(subpass);
		}
		else
		{
			for (int i = 0; i < rtLayout.subpasses.GetSize(); i++)
			{
				SubPassDescriptor subpass{};
				HashSet<u32> unusedAttachmentsThisSubpass{};
				HashSet<u32> usedAttachmentsThisSubpass{};

				for (int j = 0; j < outDescriptor.attachments.GetSize(); j++)
				{
					unusedAttachmentsThisSubpass.Add(j);
				}

				for (int j = 0; j < rtLayout.subpasses[i].colorAttachments.GetSize(); j++)
				{
					SubPassAttachment attachmentRef{};
					attachmentRef.attachmentIndex = rtLayout.subpasses[i].colorAttachments[j];
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					unusedAttachmentsThisSubpass.Remove(attachmentRef.attachmentIndex);
					usedAttachmentsThisSubpass.Add(attachmentRef.attachmentIndex);

					subpass.colorAttachments.Add(attachmentRef);
				}

				for (int j = 0; j < rtLayout.subpasses[i].depthStencilAttachment.GetSize(); j++)
				{
					SubPassAttachment attachmentRef{};
					attachmentRef.attachmentIndex = rtLayout.subpasses[i].depthStencilAttachment[j];
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					unusedAttachmentsThisSubpass.Remove(attachmentRef.attachmentIndex);
					usedAttachmentsThisSubpass.Add(attachmentRef.attachmentIndex);

					subpass.depthStencilAttachment.Add(attachmentRef);
				}

				for (int j = 0; j < rtLayout.subpasses[i].resolveAttachments.GetSize(); j++)
				{
					SubPassAttachment attachmentRef{};
					attachmentRef.attachmentIndex = rtLayout.subpasses[i].resolveAttachments[j];
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					unusedAttachmentsThisSubpass.Remove(attachmentRef.attachmentIndex);
					usedAttachmentsThisSubpass.Add(attachmentRef.attachmentIndex);

					subpass.resolveAttachments.Add(attachmentRef);
				}

				for (int j = 0; j < rtLayout.subpasses[i].subpassInputAttachments.GetSize(); j++)
				{
					SubPassAttachment attachmentRef{};
					attachmentRef.attachmentIndex = rtLayout.subpasses[i].subpassInputAttachments[j];
					attachmentRef.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					unusedAttachmentsThisSubpass.Remove(attachmentRef.attachmentIndex);
					usedAttachmentsThisSubpass.Add(attachmentRef.attachmentIndex);

					subpass.subpassInputAttachments.Add(attachmentRef);
				}

				for (u32 unusedAttachmentIndex : unusedAttachmentsThisSubpass)
				{
					// If this attachment was used in an earlier subpass but is not used in current subpass
					if (renderPassUsedAttachments.Exists(unusedAttachmentIndex))
					{
						// If we want to keep the data in this attachment intact, we need to add it to preserveAttachments
						if (outDescriptor.attachments[unusedAttachmentIndex].loadStoreAction.storeAction == RHI::AttachmentStoreAction::Store)
						{
							subpass.preserveAttachments.Add(unusedAttachmentIndex);
						}
					}
				}

				for (u32 usedAttachmentIndex : usedAttachmentsThisSubpass)
				{
					renderPassUsedAttachments.Add(usedAttachmentIndex);
				}

				outDescriptor.subpasses.Add(subpass);
			}
		}

		outDescriptor.CompileDependencies();
	}

	void RenderPass::Descriptor::CompileDependencies()
	{
		dependencies.Clear();

		for (int i = 0; i < subpasses.GetSize(); i++)
		{
			auto& subpass = subpasses[i];

			VkSubpassDependency externalDependency{};
			externalDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			externalDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			externalDependency.srcAccessMask = 0;
			externalDependency.dstSubpass = i;
			externalDependency.dstStageMask = 0;
			externalDependency.dstAccessMask = 0;

			bool usesColorAttachment = subpass.colorAttachments.GetSize() > 0;
			bool usesDepthStencilAttachment = subpass.depthStencilAttachment.GetSize() > 0;
			bool usesSubpassAttachment = subpass.subpassInputAttachments.GetSize() > 0;

			if (usesColorAttachment)
			{
				externalDependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				externalDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			if (usesDepthStencilAttachment)
			{
				externalDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				externalDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}
			if (usesSubpassAttachment)
			{
				externalDependency.dstStageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				externalDependency.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
			}

			dependencies.Add(externalDependency);

			for (int prev = 0; prev < i; prev++)
			{
				const auto& prevSubpass = subpasses[prev];

				VkSubpassDependency dependency{};
				dependency.srcSubpass = prev;
				dependency.dstSubpass = i;

				for (int idx0 = 0; idx0 < prevSubpass.colorAttachments.GetSize(); idx0++)
				{
					// Common color -> color attachment
					for (int idx1 = 0; idx1 < subpass.colorAttachments.GetSize(); idx1++)
					{
						if (prevSubpass.colorAttachments[idx0].attachmentIndex == subpass.colorAttachments[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

							dependencies.Add(dependency);
							break;
						}
					}

					// Common color -> resolve attachment
					for (int idx1 = 0; idx1 < subpass.resolveAttachments.GetSize(); idx1++)
					{
						if (prevSubpass.colorAttachments[idx0].attachmentIndex == subpass.resolveAttachments[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

							dependencies.Add(dependency);
							break;
						}
					}

					// Common color -> subpass input attachment
					for (int idx1 = 0; idx1 < subpass.subpassInputAttachments.GetSize(); idx1++)
					{
						if (prevSubpass.colorAttachments[idx0].attachmentIndex == subpass.subpassInputAttachments[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
							dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

							dependencies.Add(dependency);
							break;
						}
					}
				}

				for (int idx0 = 0; idx0 < prevSubpass.resolveAttachments.GetSize(); idx0++)
				{
					// Common resolve -> resolve attachment
					for (int idx1 = 0; idx1 < subpass.resolveAttachments.GetSize(); idx1++)
					{
						if (prevSubpass.resolveAttachments[idx0].attachmentIndex == subpass.resolveAttachments[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

							dependencies.Add(dependency);
							break;
						}
					}

					// Common resolve -> color attachment
					for (int idx1 = 0; idx1 < subpass.colorAttachments.GetSize(); idx1++)
					{
						if (prevSubpass.resolveAttachments[idx0].attachmentIndex == subpass.colorAttachments[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

							dependencies.Add(dependency);
							break;
						}
					}

					// Common resolve -> subpass input attachment
					for (int idx1 = 0; idx1 < subpass.subpassInputAttachments.GetSize(); idx1++)
					{
						if (prevSubpass.resolveAttachments[idx0].attachmentIndex == subpass.subpassInputAttachments[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
							dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

							dependencies.Add(dependency);
							break;
						}
					}
				}

				for (int idx0 = 0; idx0 < prevSubpass.depthStencilAttachment.GetSize(); idx0++)
				{
					// Common depth -> depth attachment
					for (int idx1 = 0; idx1 < subpass.depthStencilAttachment.GetSize(); idx1++)
					{
						if (prevSubpass.depthStencilAttachment[idx0].attachmentIndex == subpass.depthStencilAttachment[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = dependency.srcStageMask;
							dependency.dstAccessMask = dependency.srcAccessMask;

							dependencies.Add(dependency);
							break;
						}
					}

					// Common depth -> subpass input attachment
					for (int idx1 = 0; idx1 < subpass.subpassInputAttachments.GetSize(); idx1++)
					{
						if (prevSubpass.depthStencilAttachment[idx0].attachmentIndex == subpass.subpassInputAttachments[idx1].attachmentIndex)
						{
							dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							dependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
							dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

							dependencies.Add(dependency);
							break;
						}
					}
				}
			}
		}
	}

	RHI::ScopeAttachmentUsage RenderPass::GetAttachmentUsage(u32 attachmentIndex)
	{
		for (int i = 0; i < desc.subpasses.GetSize(); i++)
		{
			for (int j = 0; j < desc.subpasses[i].depthStencilAttachment.GetSize(); j++)
			{
				if (desc.subpasses[i].depthStencilAttachment[j].attachmentIndex == attachmentIndex)
					return RHI::ScopeAttachmentUsage::DepthStencil;
			}

			for (int j = 0; j < desc.subpasses[i].colorAttachments.GetSize(); j++)
			{
				if (desc.subpasses[i].colorAttachments[j].attachmentIndex == attachmentIndex)
					return RHI::ScopeAttachmentUsage::Color;
			}

			for (int j = 0; j < desc.subpasses[i].resolveAttachments.GetSize(); j++)
			{
				if (desc.subpasses[i].resolveAttachments[j].attachmentIndex == attachmentIndex)
					return RHI::ScopeAttachmentUsage::Resolve;
			}

			for (int j = 0; j < desc.subpasses[i].subpassInputAttachments.GetSize(); j++)
			{
				if (desc.subpasses[i].subpassInputAttachments[j].attachmentIndex == attachmentIndex)
					return RHI::ScopeAttachmentUsage::SubpassInput;
			}
		}

		return RHI::ScopeAttachmentUsage::None;
	}

	VkSampleCountFlagBits GetSampleCountFlagBits(int sampleCount)
	{
		switch (sampleCount)
		{
		case 1:
			return VK_SAMPLE_COUNT_1_BIT;
		case 2:
			return VK_SAMPLE_COUNT_2_BIT;
		case 4:
			return VK_SAMPLE_COUNT_4_BIT;
		case 8:
			return VK_SAMPLE_COUNT_8_BIT;
		case 16:
			return VK_SAMPLE_COUNT_16_BIT;
		case 32:
			return VK_SAMPLE_COUNT_32_BIT;
		case 64:
			return VK_SAMPLE_COUNT_64_BIT;
		}
		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkAttachmentLoadOp RHIAttachmentLoadActionToVk(RHI::AttachmentLoadAction loadAction)
	{
		switch (loadAction)
		{
		case RHI::AttachmentLoadAction::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case RHI::AttachmentLoadAction::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case RHI::AttachmentLoadAction::DontCare:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}

	VkAttachmentStoreOp RHIAttachmentStoreActionToVk(RHI::AttachmentStoreAction storeAction)
	{
		switch (storeAction)
		{
		case RHI::AttachmentStoreAction::DontCare:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case RHI::AttachmentStoreAction::Store:
			return VK_ATTACHMENT_STORE_OP_STORE;
		}
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}

	SIZE_T RenderPass::Descriptor::GetHash() const
	{
		SIZE_T hash = 0;
		for (const auto& attachment : attachments)
		{
			if (hash == 0)
				hash = attachment.GetHash();
			else
				CombineHash(hash, attachment);
		}
		for (const auto& subpass : subpasses)
		{
			if (hash == 0)
				hash = subpass.GetHash();
			else
				CombineHash(hash, subpass);
		}
		for (const auto& dependency : dependencies)
		{
			if (hash == 0)
				hash = CE::GetHash(dependency);
			else
				CombineHash(hash, dependency);
		}
		return hash;
	}

	SIZE_T RenderPass::SubPassDescriptor::GetHash() const
	{
		SIZE_T hash = 0;
		for (const auto& renderTargetAttachment : colorAttachments)
		{
			if (hash == 0)
				hash = renderTargetAttachment.GetHash();
			else
				hash = GetCombinedHash(hash, renderTargetAttachment.GetHash());
		}
		for (const auto& resolveAttachment : resolveAttachments)
		{
			if (hash == 0)
				hash = resolveAttachment.GetHash();
			else
				hash = GetCombinedHash(hash, resolveAttachment.GetHash());
		}
		for (const auto& subpassInputAttachment : subpassInputAttachments)
		{
			if (hash == 0)
				hash = subpassInputAttachment.GetHash();
			else
				hash = GetCombinedHash(hash, subpassInputAttachment.GetHash());
		}
		for (auto preserveAttachment : preserveAttachments)
		{
			if (hash == 0)
				hash = CE::GetHash(preserveAttachment);
			else
				hash = GetCombinedHash(hash, CE::GetHash(preserveAttachment));
		}
		for (const auto& depthStencilAttachment : depthStencilAttachment)
		{
			if (hash == 0)
				hash = depthStencilAttachment.GetHash();
			else
				hash = GetCombinedHash(hash, depthStencilAttachment.GetHash());
		}

		return hash;
	}

	SIZE_T RenderPass::SubPassAttachment::GetHash() const
	{
		if (!IsValid())
			return 0;
		SIZE_T hash = (SIZE_T)attachmentIndex;
		hash = GetCombinedHash(hash, (SIZE_T)imageLayout);
		return hash;
	}

	SIZE_T RenderPass::AttachmentBinding::GetHash() const
	{
		SIZE_T hash = (SIZE_T)format;
		hash = GetCombinedHash(hash, CE::GetHash((int)loadStoreAction.loadAction));
		hash = GetCombinedHash(hash, CE::GetHash((int)loadStoreAction.storeAction));
		if (IsStencilVkFormat(RHIFormatToVkFormat(format)))
		{
			hash = GetCombinedHash(hash, CE::GetHash((int)loadStoreAction.loadActionStencil));
			hash = GetCombinedHash(hash, CE::GetHash((int)loadStoreAction.storeActionStencil));
		}
		hash = GetCombinedHash(hash, (SIZE_T)initialLayout);
		hash = GetCombinedHash(hash, (SIZE_T)finalLayout);
		hash = GetCombinedHash(hash, (SIZE_T)multisampleState.sampleCount);
		return hash;
	}

} // namespace CE

