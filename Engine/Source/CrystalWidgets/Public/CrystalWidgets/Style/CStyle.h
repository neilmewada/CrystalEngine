#pragma once

namespace CE::Widgets
{
    ENUM(Flags)
	enum class CStateFlag : u32
	{
		Default = 0,
		On = BIT(0),
		Off = BIT(1),
		Hovered = BIT(2),
		Pressed = BIT(3),
		Focused = BIT(4),
		Horizontal = BIT(5),
		Vertical = BIT(6),
		Disabled = BIT(7),
		Enabled = BIT(8),
		Window = BIT(9),
		Collapsed = BIT(10),
		Active = BIT(11),
		Major = BIT(12),
		Minor = BIT(13)
	};
	ENUM_CLASS_FLAGS(CStateFlag);

	CRYSTALWIDGETS_API CStateFlag StateFlagsFromString(const String& string);

	ENUM()
	enum class CSubControl
	{
		None = 0,
		Icon,
		Tab,
		Selection,
		TitleBar,
		MenuBar,
		Header,
		Window,
		Frame,
		Check,
		Radio,
		Hint,
		Alternate,
		Splitter,
		ScrollBar,

		Cell,

		Any,
	};
	ENUM_CLASS_FLAGS(CSubControl);

	CRYSTALWIDGETS_API CSubControl SubControlFromString(const String& string);

	ENUM()
	enum class CStylePropertyType
	{
		None = 0,
		Opacity,
		Padding,
		Margin,
		Position,
		Display,
		Left, Top, Right, Bottom,
		ImageTint,
		Foreground, // Inherited
		Background,
        BackgroundImage,
		BackgroundSize,
		BackgroundPosition,
		BackgroundRepeat,
		BorderRadius,
		BorderWidth,
		BorderColor,
		ShadowColor,
		ShadowOffset,
		TextAlign, // Inherited
		Width,
		Height,
		MinWidth,
		MinHeight,
		MaxWidth,
		MaxHeight,
		Spacing,
		FontSize, // Inherited
		FontName, // Inherited
		Cursor, // Inherited
		WordWrap, // Inherited

		TextDecorationStyle,
		TextDecorationLine,
		TextDecorationColor,
		TextDecorationThickness,

		// FlexBox properties
		AlignContent,
		AlignItems,
		AlignSelf,
		JustifyContent,
		Flex,
		FlexBasis,
		FlexFlow,
		FlexWrap,
		FlexGrow,
		FlexShrink,
		FlexDirection,
		FlexOrder,
		RowGap,
		ColumnGap,
	};
	ENUM_CLASS_FLAGS(CStylePropertyType);

	CRYSTALWIDGETS_API CStylePropertyType StylePropertyTypeFromString(const String& string);
	CRYSTALWIDGETS_API EnumType* GetEnumTypeForProperty(CStylePropertyType property);

	ENUM()
	enum class CStylePropertyTypeFlags
	{
		None = 0,
		Inherited = BIT(0),
		NonInherited = BIT(1),
		All = Inherited | NonInherited,
	};
	ENUM_CLASS_FLAGS(CStylePropertyTypeFlags);

	ENUM()
	enum class CBackgroundSize : u8
	{
		Fill,
		Cover,
		Contain,
		Auto = Fill
	};
	ENUM_CLASS(CBackgroundSize);

	ENUM(Flags)
	enum class CBackgroundRepeat : u8
	{
		NoRepeat = 0,
		RepeatX = BIT(0),
		RepeatY = BIT(1),
		Repeat = RepeatX | RepeatY
	};
	ENUM_CLASS_FLAGS(CBackgroundRepeat);

	ENUM(Flags)
	enum class CTextDecorationLine : u8
	{
		None = 0,
		Underline = BIT(0),
		Overline = BIT(1),
		LineThrough = BIT(2)
	};
	ENUM_CLASS_FLAGS(CTextDecorationLine);

	ENUM()
	enum class CTextDecorationStyle : u8
	{
		Solid,
		Dashed,
		Dotted
	};
	ENUM_CLASS(CTextDecorationStyle);

	struct CTextDecoration
	{
		CTextDecorationLine linePosition = CTextDecorationLine::None;
		CTextDecorationStyle lineStyle = CTextDecorationStyle::Solid;
		Color lineColor = Colors::Clear;
		f32 thickness = 0;
	};

	ENUM()
	enum class CTextAlign : u8
	{
		Inherited = 0,
		TopLeft, TopCenter, TopRight,
		MiddleLeft, MiddleCenter, MiddleRight,
		BottomLeft, BottomCenter, BottomRight
	};
	ENUM_CLASS(CTextAlign);

	CRYSTALWIDGETS_API CTextAlign StringToAlignment(const String& string);

	ENUM()
	enum class COrientation
	{
		Horizontal = 0,
		Vertical
	};
	ENUM_CLASS(COrientation);

	ENUM()
	enum class CCursor : u8
	{
		Inherited = 0,
		Arrow,
		Hand,
		Edit,
		SizeTL,
		SizeBR,
		SizeTR,
		SizeBL,
		SizeH,
		SizeV,
		SizeAll,
		Wait
	};
	ENUM_CLASS(CCursor);

	inline SystemCursor ToSystemCursor(CCursor cursor)
	{
		switch (cursor)
		{
		case CCursor::Inherited:
			return SystemCursor::Default;
		case CCursor::Arrow:
			return SystemCursor::Arrow;
		case CCursor::Hand:
			return SystemCursor::Hand;
		case CCursor::Edit:
			return SystemCursor::IBeam;
		case CCursor::SizeTL:
		case CCursor::SizeBR:
			return SystemCursor::SizeTopLeft;
		case CCursor::SizeTR:
		case CCursor::SizeBL:
			return SystemCursor::SizeTopRight;
		case CCursor::SizeH:
			return SystemCursor::SizeHorizontal;
		case CCursor::SizeV:
			return SystemCursor::SizeVertical;
		case CCursor::SizeAll:
			return SystemCursor::SizeAll;
		case CCursor::Wait:
			return SystemCursor::Wait;
		}
		return SystemCursor::Default;
	}

	ENUM()
	enum class CWordWrap : u8
	{
		Inherited = 0,
		Normal,
		Clip,
		// WARNING: Not implemented yet
		BreakWord
	};
	ENUM_CLASS(CWordWrap);

	ENUM()
	enum class CGradientType : u8
	{
		None = 0,
		LinearGradient,
	};
	ENUM_CLASS(CGradientType);

	STRUCT()
	struct CRYSTALWIDGETS_API CGradientKey
	{
		CE_STRUCT(CGradientKey)
	public:

		FIELD()
		Color color{};

		FIELD()
		f32 position = 0;

		FIELD()
		bool isPercent = false;

	};

	STRUCT()
	struct CRYSTALWIDGETS_API CGradient
	{
		CE_STRUCT(CGradient)
	public:

		CGradient& WithRotation(f32 degrees);
		CGradient& WithType(CGradientType gradientType);
		CGradient& AddKey(const Color& color, f32 position, bool isPercent = true);

		FIELD()
		f32 rotationInDegrees = 0;

		FIELD()
		CGradientType gradientType = CGradientType::None;

		FIELD()
		Array<CGradientKey> keys{};
	};

	STRUCT()
	struct CRYSTALWIDGETS_API CStyleValue
	{
		CE_STRUCT(CStyleValue)
	public:

		enum ValueType : int
		{
			Type_None = 0,
			Type_Enum,
			Type_Single,
			Type_Vector,
			Type_Color,
			Type_String,
			Type_Gradient,
		};

		enum EnumValue : int
		{
			None = 0,
			Auto,
			Inherited,
			Initial,
			Percent,
			Absolute,
		};

		CStyleValue();
		void Release();

		template<typename TEnum> requires TIsEnum<TEnum>::Value
		CStyleValue(TEnum enumValue) : enumValue(Vec4i(1, 1, 1, 1)* (int)enumValue), valueType(Type_Enum)
		{
			enumTypeId = TYPEID(TEnum);
		}

		template<typename TEnum> requires TIsEnum<TEnum>::Value
		CStyleValue(TVector4<TEnum> enumValue4) : enumValue(Vec4i(enumValue4.x, enumValue4.y, enumValue4.z, enumValue4.w)), valueType(Type_Enum)
		{
			enumTypeId = TYPEID(TEnum);
		}

		template<>
		CStyleValue(EnumValue value) : enumValue(Vec4i(1, 1, 1, 1) * (int)value), valueType(Type_Enum)
		{
			enumTypeId = TYPEID(EnumValue);
		}

		CStyleValue(f32 single, bool isPercent = false);
		CStyleValue(const Vec4& vector, bool isPercent = false);
		CStyleValue(const Color& color);
		CStyleValue(const String& string);
		CStyleValue(const CGradient& gradient);

		CStyleValue(const CStyleValue& copy);

		CStyleValue& operator=(const CStyleValue& copy);

		CStyleValue(CStyleValue&& move) noexcept;

		void CopyFrom(const CStyleValue& copy);

		bool IsOfType(ValueType checkType) const { return valueType == checkType; }

		bool IsPercentValue() const { return enumValue == Vec4i(1, 1, 1, 1) * Percent; }
		bool IsAutoValue() const { return IsEnum() && enumValue == Vec4i(1, 1, 1, 1) * Auto; }

		bool IsValid() const { return valueType != Type_None; }
		bool IsEnum() const { return IsOfType(Type_Enum); }
		bool IsSingle() const { return IsOfType(Type_Single); }
		bool IsVector() const { return IsOfType(Type_Vector); }
		bool IsColor() const { return IsOfType(Type_Color); }
		bool IsString() const { return IsOfType(Type_String); }
		bool IsGradient() const { return IsOfType(Type_Gradient); }

		bool operator==(const CStyleValue& rhs) const
		{
			if (valueType != rhs.valueType)
				return false;
			if (IsSingle() && single != rhs.single)
				return false;
			if (IsVector() && vector != rhs.vector)
				return false;
			if (IsColor() && color.ToVec4() != rhs.color.ToVec4())
				return false;
			if (IsString() && string != rhs.string)
				return false;
			if (enumValue != rhs.enumValue)
				return false;
			return true;
		}

		bool operator!=(const CStyleValue& rhs) const
		{
			return !(*this == rhs);
		}

		// Modifiers

		CStyleValue& AsAbsolute()
		{
			enumValue = Vec4i(Absolute, Absolute, Absolute, Absolute);
			return *this;
		}

		CStyleValue& AsPercent()
		{
			enumValue = Vec4i(Percent, Percent, Percent, Percent);
			return *this;
		}

		CStyleValue& AsPercent(bool maskX, bool maskY, bool maskZ, bool maskW)
		{
			if (maskX) enumValue.x = Percent;
			if (maskY) enumValue.y = Percent;
			if (maskZ) enumValue.z = Percent;
			if (maskW) enumValue.w = Percent;
			return *this;
		}

		CStyleValue& AsAuto()
		{
			enumValue = Vec4i(Auto, Auto, Auto, Auto);
			return *this;
		}

		CStyleValue& AsAuto(bool maskX, bool maskY, bool maskZ, bool maskW)
		{
			if (maskX) enumValue.x = Auto;
			if (maskY) enumValue.y = Auto;
			if (maskZ) enumValue.z = Auto;
			if (maskW) enumValue.w = Auto;
			return *this;
		}

		CStyleValue& WithState(CStateFlag state)
		{
			this->state = state;
			return *this;
		}

		CStyleValue& WithSubControl(CSubControl subControl)
		{
			this->subControl = subControl;
			return *this;
		}

		FIELD()
		f32 single = 0;

		FIELD()
		Vec4 vector{};

		FIELD()
		Color color = Color();

		FIELD()
		String string = "";

		FIELD()
		Vec4i enumValue{};

		FIELD()
		CStateFlag state{}; // Style for a single specific state

		FIELD()
		CSubControl subControl{}; // Style for a specific subcontrol

		FIELD()
		CGradient gradient{};

		FIELD()
		int valueType = 0;

		TypeId enumTypeId = 0;
	};

	struct CRYSTALWIDGETS_API CStyle
	{
	public:

		static const Array<CStylePropertyType>& GetInheritedProperties();
		static bool IsInheritedProperty(CStylePropertyType property);
		static TypeInfo* GetValueTypeForProperty(CStylePropertyType property);
		static bool IsLayoutProperty(CStylePropertyType property);
		static CStylePropertyTypeFlags GetPropertyTypeFlags(CStylePropertyType property);

		void ApplyProperties(const CStyle& from);

		//! Returns true if layout properties are different, else false.
		bool CompareLayoutProperties(const CStyle& rhs);

		Vec4 GetPadding() const
		{
			if (!properties.KeyExists(CStylePropertyType::Padding))
				return Vec4();
			return properties.Get(CStylePropertyType::Padding).vector;
		}

		Name GetFontName() const
		{
			if (!properties.KeyExists(CStylePropertyType::FontName))
				return "Roboto";
			String fontName = properties.Get(CStylePropertyType::FontName).string;
			if (fontName.IsEmptyOrWhiteSpace())
				return "Roboto";
			return fontName;
		}

		f32 GetFontSize() const
		{
			if (!properties.KeyExists(CStylePropertyType::FontSize))
				return 14;
			f32 fontSize = properties.Get(CStylePropertyType::FontSize).single;
			if (fontSize < 1)
				fontSize = 14;
			return fontSize;
		}

		Color GetForegroundColor() const
		{
			if (!properties.KeyExists(CStylePropertyType::Foreground))
				return Colors::White;
			return properties.Get(CStylePropertyType::Foreground).color;
		}

		Color GetBackgroundColor() const
		{
			if (!properties.KeyExists(CStylePropertyType::Background))
				return Colors::Clear;
			return properties.Get(CStylePropertyType::Background).color;
		}

		Vec4 GetBorderRadius() const
		{
			if (!properties.KeyExists(CStylePropertyType::BorderRadius))
				return Vec4();
			return properties.Get(CStylePropertyType::BorderRadius).vector;
		}

		CBackgroundSize GetBackgroundSize() const
		{
			if (!properties.KeyExists(CStylePropertyType::BackgroundSize))
				return CBackgroundSize::Auto;
			return (CBackgroundSize)properties.Get(CStylePropertyType::BackgroundSize).enumValue.x;
		}

		CTextAlign GetBackgroundPosition() const
		{
			if (!properties.KeyExists(CStylePropertyType::BackgroundPosition))
				return CTextAlign::MiddleCenter;
			return (CTextAlign)properties.Get(CStylePropertyType::BackgroundPosition).enumValue.x;
		}

		CBackgroundRepeat GetBackgroundRepeat() const
		{
			if (!properties.KeyExists(CStylePropertyType::BackgroundRepeat))
				return CBackgroundRepeat::NoRepeat;
			return (CBackgroundRepeat)properties.Get(CStylePropertyType::BackgroundRepeat).enumValue.x;
		}

		CWordWrap GetWordWarp() const
		{
			if (!properties.KeyExists(CStylePropertyType::BackgroundPosition))
				return CWordWrap::Normal;
			return (CWordWrap)properties.Get(CStylePropertyType::WordWrap).enumValue.x;
		}

		CTextDecoration GetTextDecoration() const
		{
			CTextDecoration result{};

			if (properties.KeyExists(CStylePropertyType::TextDecorationStyle))
				result.lineStyle = (CTextDecorationStyle)properties.Get(CStylePropertyType::TextDecorationStyle).enumValue.x;

			if (properties.KeyExists(CStylePropertyType::TextDecorationLine))
				result.linePosition = (CTextDecorationLine)properties.Get(CStylePropertyType::TextDecorationLine).enumValue.x;

			if (properties.KeyExists(CStylePropertyType::TextDecorationThickness))
				result.thickness = properties.Get(CStylePropertyType::TextDecorationThickness).single;

			if (properties.KeyExists(CStylePropertyType::TextDecorationColor))
				result.lineColor = properties.Get(CStylePropertyType::TextDecorationColor).color;
			else
				result.lineColor = GetForegroundColor();

			return result;
		}

		HashMap<CStylePropertyType, CStyleValue> properties{};

	};

} // namespace CE::Widgets

#include "CStyle.rtti.h"