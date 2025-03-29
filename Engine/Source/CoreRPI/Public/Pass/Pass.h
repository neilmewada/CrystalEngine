#pragma once

#if PAL_TRAIT_BUILD_TESTS
class RenderPipeline_DefaultPipelineTree_Test;
#endif

#define RPI_PASS(passName)\
	friend class Pass;\
	friend class PassTree;\
	friend class ParentPass;\
	friend class GpuPass;\
	friend class PassLibrary;\
	friend class PassSystem;\
	friend class RenderPipeline;

namespace CE::RPI
{
	class ParentPass;
	class RenderPipeline;
	
	/// @brief The base Pass class. All passes should derive from this class.
	CLASS(Abstract)
	class CORERPI_API Pass : public Object, public RHI::IScopeProducer
	{
		CE_CLASS(Pass, Object)
	public:

		virtual ~Pass();

		virtual bool IsParentPass() const { return false; }

		void AddAttachmentBinding(const PassAttachmentBinding& attachmentBinding);

		void AddSlot(const PassSlot& slot);

		PassSlot* FindSlot(const Name& name);

		PassAttachmentBinding* FindInputBinding(const Name& name);
		PassAttachmentBinding* FindInputOutputBinding(const Name& name);
		PassAttachmentBinding* FindOutputBinding(const Name& name);

		PassAttachmentBinding* FindBinding(const Name& name);

		const Name& GetPassName() const override { return GetName(); }

		RHI::DrawListTag GetDrawListTag() const { return drawListTag; }

		void SetDrawListTag(RHI::DrawListTag tag) { drawListTag = tag; }

		SceneViewTag GetViewTag() const { return viewTag; }

		void SetViewTag(const SceneViewTag& tag) { viewTag = tag; }

		Name GetScopeId() const;

		//const RHI::ShaderResourceGroupLayout& GetPerPassSrgLayout() const { return perPassSrgLayout; }

		const Array<PassAttachmentBinding>& GetInputBindings() const { return inputBindings; }
		const Array<PassAttachmentBinding>& GetInputOutputBindings() const { return inputOutputBindings; }
		const Array<PassAttachmentBinding>& GetOutputBindings() const { return outputBindings; }

	protected:

		void ProduceScopes(RHI::FrameScheduler* scheduler) override {}

		void EmplaceAttachments(RHI::FrameScheduler* scheduler) override {}

		Pass();

		/// @brief Draw list tag this pass is associated to.
		RHI::DrawListTag drawListTag{};
		
		/// @brief The view tag associated with a view.
		/// The view that matches this tag will be queried by this pass.
		SceneViewTag viewTag{};

		RPI::RenderPipeline* renderPipeline = nullptr;

		FIELD()
		ParentPass* parentPass = nullptr;

		FIELD()
		Array<PassSlot> slots{};

		FIELD()
		RHI::ShaderResourceGroupLayout perPassSrgLayout{};

		Array<PassAttachmentBinding> inputBindings{};
		Array<PassAttachmentBinding> inputOutputBindings{};
		Array<PassAttachmentBinding> outputBindings{};

		friend class RenderPipeline;

		RPI_PASS(Pass);

#if PAL_TRAIT_BUILD_TESTS
		friend class RenderPipeline_DefaultPipelineTree_Test;
#endif
	};

    
} // namespace CE::RPI

#include "Pass.rtti.h"
