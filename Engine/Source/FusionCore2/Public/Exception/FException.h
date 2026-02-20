#pragma once

namespace CE
{
    
    class FUSIONCORE_API FException : public CE::Exception
	{
	public:

		FException(const String& message, WeakRef<Object> sender = nullptr) : Exception(message), sender(sender)
		{}

		Ref<Object> GetSender() const { return sender.Lock(); }

	private:

		WeakRef<Object> sender;

	};

} // namespace CE
