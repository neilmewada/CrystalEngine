#pragma once

enum AttributeSpecifiers
{
	//! @brief Marks the class as abstract.
	Abstract,

	//! @brief Set the user-friendly name for the editor.
	DisplayName,

	SettingsCategory,

	//! @brief Set the minimum numeric value of the field.
	RangeMin,

	//! @brief Set the maximum numeric value of the field.
	RangeMax,

	//! @brief Set the category of the field.
	Category,

	//! @brief Set the order of the category.
	CategoryOrder,

	//! @brief Set array editor mode: Default, Static.
	//! Default: Can edit the array in any way possible.
	//! Static: Cannot add or delete elements, but can edit existing elements.
	ArrayEditorMode,

	//! @brief A macro to customize Name display for each array element.
	//! Instead of the generic "Index 0", you can specify ArrayElementName = "Item {}"
	//! to make it show "Item 0" for the element at 0th index.
	ArrayElementName,

	//! @brief A macro to customize TypeName display for each array element.
	ArrayElementTypeName,

	//! @brief A macro to customize TypeName display for a struct field.
	StructTypeName,

	//! @brief A macro to disable visibility of an editor property based on a sibling field's value.
	//! Example: FIELD(EditAnywhere, ShowIf = ".propertyType == UInt")
	ShowIf,

	//! @brief Indicates that the field is editable anywhere.
	EditAnywhere,

	//! @brief This makes the field or button visible anywhere, but not editable.
	VisibleAnywhere,

	//! @brief Indicates that the field is read-only.
	ReadOnly,

	//! @brief This will exclude the field from serialization.
	NonSerialized,

	//! @brief Mark the property as a 'regular' fusion widget property
	FusionProperty,

	//! @brief Mark the property as fusion widget layout property.
	FusionLayoutProperty,

	//! @brief Mark the property as fusion widget data property
	FusionDataProperty,

	//! @brief Internal use only! Used to mark FPropertyBinding fields.
	FusionDataBinding,

	//! @brief Internal use only! Used to mark methods like Update_Text() which triggers a read.
	FusionDataUpdate,

	ModelProperty,

	ModelPropertyEditable
};

/*
 * Used to mark a class as reflected.
 * Attribute Usage:
 *  Abstract // Must be set if the class is either abstract or doesn't have default constructor
 *  DisplayName = "Struct Display Name" // Set the display name of the class
 */
#define CLASS(...)

/*
 * Used to mark a struct as reflected.
 * Attribute Usage:
 *  DisplayName = "Struct Display Name" // Set the display name of the struct
 */
#define STRUCT(...)


/*
 * Used to mark an enum as reflected.
 * Attribute Usage:
 *  DisplayName = "Enum Display Name" // Set the display name of the enum
 */
#define ENUM(...)

/*
 * Used to set attributes for a constant inside a reflected enum
 * Attribute Usage:
 *  DisplayName = "Enum Constant Display Name" // Set the display name of the enum constant
 */
#define ECONST(...)

/*
 * Used to mark a field as reflected.
 */
#define FIELD(...)

/*
 * Used to mark a function as reflected.
 * Attribute Usage:
 *  Event // Mark the function as an event, to be bound to a signal or an event bus
 */
#define FUNCTION(...)

#define EVENT(...)
