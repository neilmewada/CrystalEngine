#pragma once

namespace CE::RPI
{
	class Pass;

	/// @brief Describes the attachment used by a pass
	struct CORERPI_API PassAttachment final : IntrusiveBase
	{
	public:

		PassAttachment(const PassImageAttachmentDesc& desc);
		PassAttachment(const PassBufferAttachmentDesc& desc);

		/// @brief Name of the attachment.
		Name name{};

		//! @brief Actual attachment id to use in Frame Scheduler.
		RHI::AttachmentID attachmentId = "";

		RHI::AttachmentLifetimeType lifetime = RHI::AttachmentLifetimeType::Transient;
        
        PassAttachmentSizeSource sizeSource{};
        
        RPI::UnifiedAttachmentDescriptor attachmentDescriptor{};

		Name formatSource = "";

		Array<RHI::Format> fallbackFormats{};
	};

	/// @brief A Pass attachment binding either points to a pass attachment OR to another
	/// pass attachment binding, for example, an input slot of one pass connected to output of another.
	struct CORERPI_API PassAttachmentBinding final
	{
	public:
        
		/// @brief Name of the attachment binding. Either pass slot name, or attachment name.
		Name name = "";
        
		PassSlotType slotType = PassSlotType::Undefined;
        
		RHI::ScopeAttachmentUsage attachmentUsage = RHI::ScopeAttachmentUsage::None;
        
		/// @brief Pointer to another attachment binding slot this binding is connected to.
		PassAttachmentBinding* connectedBinding = nullptr;
        
        /// @brief Pointer to the binding when the pass that owns this binding is disabled. Used only for output slot types.
		PassAttachmentBinding* fallbackBinding = nullptr;

		/// @brief The pass that owns this binding.
		Pass* ownerPass = nullptr;
        
		/// @brief The pass attachment this binding points to. connectedBinding should be null if this is used.
		Ptr<PassAttachment> attachment = nullptr;

		Ptr<PassAttachment> GetOriginalAttachment() const;

	private:

		mutable Ptr<PassAttachment> originalAttachment = nullptr;
	};

} // namespace CE::RPI

