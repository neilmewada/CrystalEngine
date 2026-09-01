#pragma once

#include <optional>
#include <utility>

namespace CE
{

	struct NullOptType
	{
		explicit constexpr NullOptType() = default;
	};

	inline constexpr NullOptType NullOpt{};

	template<typename T>
	class Optional
	{
	public:
		constexpr Optional() = default;
		constexpr Optional(NullOptType) {}

		Optional(const T& value) : impl(value) {}
		Optional(T&& value) : impl(std::move(value)) {}

		Optional(const Optional&) = default;
		Optional(Optional&&) noexcept = default;
		Optional& operator=(const Optional&) = default;
		Optional& operator=(Optional&&) noexcept = default;

		Optional& operator=(NullOptType)
		{
			Reset();
			return *this;
		}

		Optional& operator=(const T& value)
		{
			impl = value;
			return *this;
		}

		Optional& operator=(T&& value)
		{
			impl = std::move(value);
			return *this;
		}

		CE_INLINE bool HasValue() const
		{
			return impl.has_value();
		}

		CE_INLINE explicit operator bool() const
		{
			return HasValue();
		}

		CE_INLINE T& GetValue()
		{
			return impl.value();
		}

		CE_INLINE const T& GetValue() const
		{
			return impl.value();
		}

		template<typename U>
		CE_INLINE T ValueOr(U&& fallback) const
		{
			return impl.value_or(std::forward<U>(fallback));
		}

		template<typename... Args>
		CE_INLINE T& Emplace(Args&&... args)
		{
			return impl.emplace(std::forward<Args>(args)...);
		}

		CE_INLINE void Reset()
		{
			impl.reset();
		}

		CE_INLINE T* operator->()
		{
			return &GetValue();
		}

		CE_INLINE const T* operator->() const
		{
			return &GetValue();
		}

		CE_INLINE T& operator*()
		{
			return GetValue();
		}

		CE_INLINE const T& operator*() const
		{
			return GetValue();
		}

	private:
		std::optional<T> impl;
	};

} // namespace CE
