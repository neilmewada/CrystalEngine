#include "FusionCore.h"

namespace CE
{
	FStyleSet::FStyleSet()
	{
		
	}

	Ref<FStyle> FStyleSet::FindStyle(CE::Name name)
	{
		if (stylesByKey.KeyExists(name))
			return stylesByKey[name];
		if (Ref<FStyleSet> parent = this->parent.Lock())
			return parent->FindStyle(name);
		return nullptr;
	}

	void FStyleSet::SetParent(Ref<FStyleSet> parent)
	{
		this->parent = parent;
	}

	void FStyleSet::RegisterStyle(CE::Name name, Ref<FStyle> style)
	{
		stylesByKey[name] = style;
		style->parent = this;
	}

	void FStyleSet::DeregisterStyle(CE::Name name)
	{
		auto it = stylesByKey.Find(name);

		if (it != stylesByKey.End())
		{
			if (it->second.IsValid() && it->second->parent.Get() == this)
				it->second->parent = nullptr;

			stylesByKey.Remove(name);
		}
	}

	void FStyleSet::InitDefault()
	{
		// -------------------------------------------------------------------
		// Button/Primary — filled accent, main call-to-action
		// -------------------------------------------------------------------
		{
			auto style = CreateObject<FButtonStyle>(this, "Button_Primary");

			style->backgroundDefault  = FBrush(Color(0.23f, 0.48f, 0.96f));
			style->backgroundHovered  = FBrush(Color(0.35f, 0.56f, 1.00f));
			style->backgroundPressed  = FBrush(Color(0.16f, 0.38f, 0.85f));
			style->backgroundDisabled = FBrush(Color(0.24f, 0.31f, 0.43f));
			style->shape              = FRoundedRectangle(4.0f);

			RegisterStyle("Button/Primary", style);
		}

		// -------------------------------------------------------------------
		// Button/Secondary — outlined with solid fills, secondary actions
		// -------------------------------------------------------------------
		{
			auto style = CreateObject<FButtonStyle>(this, "Button_Secondary");

			style->backgroundDefault  = FBrush(Color(0.14f, 0.14f, 0.17f));
			style->backgroundHovered  = FBrush(Color(0.20f, 0.20f, 0.26f));
			style->backgroundPressed  = FBrush(Color(0.11f, 0.11f, 0.14f));
			style->backgroundDisabled = FBrush(Color(0.14f, 0.14f, 0.17f));

			style->borderDefault      = FPen(Color(0.23f, 0.48f, 0.96f), 1.0f);
			style->borderHovered      = FPen(Color(0.35f, 0.56f, 1.00f), 1.0f);
			style->borderPressed      = FPen(Color(0.16f, 0.38f, 0.85f), 1.0f);
			style->borderDisabled     = FPen(Color(0.28f, 0.28f, 0.33f), 1.0f);

			style->shape              = FRoundedRectangle(4.0f);

			RegisterStyle("Button/Secondary", style);
		}

		// -------------------------------------------------------------------
		// Button/Destructive — red fill, irreversible actions
		// -------------------------------------------------------------------
		{
			auto style = CreateObject<FButtonStyle>(this, "Button_Destructive");

			style->backgroundDefault  = FBrush(Color(0.75f, 0.17f, 0.13f));
			style->backgroundHovered  = FBrush(Color(0.88f, 0.22f, 0.17f));
			style->backgroundPressed  = FBrush(Color(0.62f, 0.12f, 0.09f));
			style->backgroundDisabled = FBrush(Color(0.38f, 0.20f, 0.19f));
			style->shape              = FRoundedRectangle(4.0f);

			RegisterStyle("Button/Destructive", style);
		}
	}
} // namespace CE
