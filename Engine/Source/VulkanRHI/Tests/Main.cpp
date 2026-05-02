
#include "CoreRHI.h"
#include "VulkanRHI.h"

#include <gtest/gtest.h>

using namespace CE;

static void TestBegin(bool gui = false)
{
	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CoreApplication");
	ModuleManager::Get().LoadModule("CoreMedia");
	ModuleManager::Get().LoadModule("CoreRHI");

	ModuleManager::Get().LoadModule("VulkanRHI");
	RHI::gDynamicRHI = new Vulkan::VulkanRHI;

	RHI::gDynamicRHI->Initialize();
	RHI::gDynamicRHI->PostInitialize();

	RHI::FrameSchedulerDescriptor desc{ 2 };

	[[maybe_unused]] RHI::FrameScheduler* scheduler = RHI::FrameScheduler::Create(desc);
}

static void TestEnd()
{
	auto scheduler = RHI::FrameScheduler::Get();
	delete scheduler;

	RHI::gDynamicRHI->PreShutdown();
	RHI::gDynamicRHI->Shutdown();

	delete RHI::gDynamicRHI; RHI::gDynamicRHI = nullptr;

	ModuleManager::Get().UnloadModule("VulkanRHI");
	ModuleManager::Get().UnloadModule("CoreRHI");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("Core");
}

#define TEST_BEGIN(...) using namespace RHI; TestBegin(__VA_ARGS__)
#define TEST_END() TestEnd()

static void BuildSampleRenderPipeline(RHI::FrameScheduler* scheduler)
{
	using namespace RHI;

	scheduler->BeginFrameGraph();
	{
		auto& attachmentDatabase = scheduler->GetAttachmentDatabase();

		const u32 kWidth = 1920, kHeight = 1080;

		const Name kDepthAttachment = "_Depth";
		attachmentDatabase.EmplaceFrameAttachment(kDepthAttachment, RHI::ImageDescriptor{
			.name = kDepthAttachment,
			.width = kWidth,
			.height = kHeight,
			.depth = 1,
			.dimension = Dimension::Dim2D,
			.format = Format::D32_SFLOAT,
			.mipLevels = 1,
			.sampleCount = 1,
			.arrayLayers = 1,
			.bindFlags = TextureBindFlags::Depth,
			.defaultHeapType = MemoryHeapType::Default
			});

		const Name kShadowMapAttachment = "_ShadowMap";
		attachmentDatabase.EmplaceFrameAttachment(kShadowMapAttachment, RHI::ImageDescriptor{
			.name = kShadowMapAttachment,
			.width = 1024,
			.height = 1024,
			.depth = 1,
			.dimension = Dimension::Dim2D,
			.format = Format::D32_SFLOAT,
			.mipLevels = 1,
			.sampleCount = 1,
			.arrayLayers = 1,
			.bindFlags = TextureBindFlags::Depth,
			.defaultHeapType = MemoryHeapType::Default
			});

		const Name kColorOutputAttachment = "_ShadowMap";
		attachmentDatabase.EmplaceFrameAttachment(kColorOutputAttachment, RHI::ImageDescriptor{
			.name = kColorOutputAttachment,
			.width = kWidth,
			.height = kHeight,
			.depth = 1,
			.dimension = Dimension::Dim2D,
			.format = Format::D32_SFLOAT,
			.mipLevels = 1,
			.sampleCount = 1,
			.arrayLayers = 1,
			.bindFlags = TextureBindFlags::Depth,
			.defaultHeapType = MemoryHeapType::Default
			});

		const Name kTiledLightList = "_TiledLightList";
		attachmentDatabase.EmplaceFrameAttachment(kTiledLightList, RHI::BufferDescriptor{
			.name = kTiledLightList,
			.bufferSize = 1_MB,
			.structureByteStride = 1_MB,
			.alignment = 0,
			.bindFlags = BufferBindFlags::StructuredBuffer,
			.defaultHeapType = MemoryHeapType::Default
		});

		scheduler->BeginScope("Depth");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Shadow");
		{
			ImageScopeAttachmentDescriptor shadowAttachment{};
			shadowAttachment.attachmentId = kShadowMapAttachment;

			scheduler->UseAttachment(shadowAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Skybox");
		{
			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;
			
			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("TileCulling", ScopeOperation::Compute, HardwareQueueClass::Compute);
		{
			BufferScopeAttachmentDescriptor lightListAttachment{};
			lightListAttachment.attachmentId = kTiledLightList;
			lightListAttachment.shaderInputName = "_LightList";

			scheduler->UseAttachment(lightListAttachment, ScopeAttachmentUsage::Shader, ScopeAttachmentAccess::Write);

			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Opaque");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;
			
			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;

			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::ReadWrite);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Transparent");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;

			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::ReadWrite);
		}
		scheduler->EndScope();
	}
	scheduler->EndFrameGraph();
}

TEST(FrameGraphTest, Build)
{
	TEST_BEGIN(true);

	auto scheduler = RHI::FrameScheduler::Get();

	BuildSampleRenderPipeline(scheduler);

	for (int t = 0; t <= scheduler->GetMaxTimelineLevel(); t++)
	{
		Array scopes = scheduler->GetScopesAtTimelineLevel(t);

		if (t == 0)
		{
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Depth"; }));
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Shadow"; }));
		}
		else if (t == 1)
		{
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Skybox"; }));
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "TileCulling" && s->GetOperation() == ScopeOperation::Compute; }));
		}
		else if (t == 2)
		{
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Opaque"; }));
		}
		else if (t == 3)
		{
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Transparent"; }));
		}
	}

	scheduler->Compile();
	
	TEST_END();
}

