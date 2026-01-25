# Bundle File Format Specification

The `Bundle` class is used by the engine to store & load assets & serialized objects. It stores the data in a binary file with `.casset` extension.

This document describes the file format specification of `.casset` files. The current format version is 3.1.

# Format Specification


### Basic Types

| Type | Value | Description |
|---|---|---|
| String | `T e x t 00` | Null terminated string |

## **File Structure**
| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 8B | `00 42 55 4e 44 4c 45 00` | Magic Number: `. B U N D L E .` |
| +08 | 4B | `00 00 00 00` | Header checksum |
| +0C | 4B | 3 | Major Version (u32) |
| +10 | 4B | 1 | Minor Version (u32) |
| +14 | 4B | 0 | Patch Version (u32) (Unused) |
| +1C | 8B | `00 00 00 00 00 00 00 00` | [Schema Table](#schema-table) start offset (from start of file) |
| +24 | 8B | `00 00 00 00 00 00 00 00` | [Serialized Data](#serialized-data) start offset (from start of file) |
|  | | **<-- Header fields start below -->** |  |
| +2C | 16B | 128 bit Uuid | Bundle UUID |
| +3C | \0 | `/Game/Assets/MyTextureAsset\0` | Full path/name of bundle (String) |
| +xx | 4B | `01 00 00 00` | No. of external bundle dependencies |
| - | - | *bundle dependency list (repeat Uuid)* | - |
| +xx | 16B | 128 bit Uuid | n-th Bundle dependency (Uuid). |
| +10 | 1B | `00/01` | Is cooked? |
| +11 | \0 | `/Path/To/Source/Asset.png` | Relative path to the source asset (v3.1) |
| - | - | *newly added header fields go here* | - |
| - | - | *schema table goes here* | - |
| - | - | *serialized data goes here* | - |
| +xx | 8B | `00 00 00 00 00 00 00 00` | EOF - End of file |


## **Schema Table**

Schema table stores the layout of each object and struct that is serialized in this bundle.

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 8B | `xx xx xx xx xx xx xx xx` | Size of this table in bytes (including this field and End of schema table) |
| +08 | 4B | N | Number of entries in schema |
| | | | |
| | | | |
|  |  | **<-- Entry #0 -->** |  |
| +0C | 4B | `xx xx xx xx` | Size of 1-st schema entry in bytes. (including this field) |
| +10 | 1B | `00/01` | 0 = Class ; 1 = Struct ; 2 = Opaque POD ; |
| +11 | \0 | `/Code/Core.CE::SomeClass\0` | Full type name of the class or struct or Opaque POD |
| +xx | 4B | `00 00 00 00` | Class/Struct Version (Unused) (Removed for OpaquePOD) |
| +04 | 4B | n1 | Number of fields (Removed for OpaquePOD) |
|  |  | Field #0 |  |
| +08 | \0 | `fieldName0` | Name of the field as in C++ (Removed for OpaquePOD) |
| +xx | 1B | `xx` | [Field Type](#field-type) byte. Indicates the field's data type. (Removed for OpaquePOD) |
| +xx | 4B | 0-based index into [Schema Table](#schema-table) | (**Optional_1**) Present only if [Field Type](#field-type) byte requires. (Removed for OpaquePOD) |
| +xx | 1B | `xx` Underlying [Field Type](#field-type) byte | (**Optional_2**) Present only if [Field Type](#field-type) byte requires. (Removed for OpaquePOD) |
|  |  | Field #1 |  |
| +xx | \0 | `fieldName1` | Name of the field as in C++ (Removed for OpaquePOD) |
| +xx | 1B | `xx` | [Field Type](#field-type) byte. Indicates the field's data type. (Removed for OpaquePOD) |
| +xx | 4B | 0-based index into [Schema Table](#schema-table) | (**Optional_1**) Present only if [Field Type](#field-type) byte requires. (Removed for OpaquePOD) |
| +xx | 1B | `xx` Underlying [Field Type](#field-type) byte | (**Optional_2**) Present only if [Field Type](#field-type) byte requires. (Removed for OpaquePOD) |
| | | | |
| | | | |
|  |  | **<-- Entry #1 -->** |  |
| +xx | 4B | `xx xx xx xx` | Size of 2-nd schema entry in bytes. (including this field) |
| +04 | 1B | `00/01` | 0 = Class ; 1 = Struct |
| +05 | \0 | `/Code/Core.CE::SomeStruct\0` | Full type name of the class or struct |
| +xx | 4B | `00 00 00 00` | Class/Struct Version (Unused) |
| +04 | 4B | n2 | Number of fields |
|  |  | Field #0 |  |
| +08 | \0 | `fieldName0` | Name of the field as in C++ |
| +xx | 1B | `xx` | [Field Type](#field-type) byte. Indicates the field's data type. |
| +xx | 4B | 0-based index into [Schema Table](#schema-table) | (**Optional_1**) Present only if [Field Type](#field-type) byte requires.  |
| +xx | 1B | `xx` Underlying [Field Type](#field-type) byte | (**Optional_2**) Present only if [Field Type](#field-type) byte requires. |
|  |  | Field #1 |  |
| +xx | \0 | `fieldName1` | Name of the field as in C++ |
| +xx | 1B | `xx` | [Field Type](#field-type) byte. Indicates the field's data type. |
| +xx | 4B | 0-based index into [Schema Table](#schema-table) | (**Optional_1**) Present only if [Field Type](#field-type) byte requires.  |
| +xx | 1B | `xx` Underlying [Field Type](#field-type) byte | (**Optional_2**) Present only if [Field Type](#field-type) byte requires. |
| | | | |
| | | | |
|  |  | **<-- Repeating segment ends here -->** |  |
| | 4B | `00 00 00 00` | **End** of schema table |

### Field Type

| Type byte | Type | Description | Requirements in Schema |
|---|---|---|---|
| `00` | null | Null value |
| `01` | u8 | uint8 |
| `02` | u16 | uint16 |
| `03` | u32 | uint32 |
| `04` | u64 | uint64 |
| `05` | s8 | int8 |
| `06` | s16 | int16 |
| `07` | s32 | int32 |
| `08` | s64 | int64 |
| `09` | f32 | float |
| `0A` | f64 | double |
| `0B` | b8 | boolean |
| `0C` | String | String |
| `0D` | Vec2 | Vec2 |
| `0E` | Vec3 | Vec3 |
| `0F` | Vec4 | Vec4 |
| `10` | Vec2i | Vec2i |
| `11` | Vec3i | Vec3i |
| `12` | Vec4i | Vec4i |
| `13` | Array\<Object\> | [Array of Objects](#array-of-objects) |
| `14` | Array\<Struct\> | Array of struct types | **Optional_1** |
| `15` | Array\<[FieldType](#field-type)\> | Array of simple types | **Optional_2** |
| `16` | [Binary](#binary-data-type) | Raw binary data |
| `17` | [Object Ref](#object-reference) | Object reference |
| `18` | [Function Binding](#function-binding) | Function binding to an object (ScriptDelegate). |
| `19` | [Array\<Function Binding\>](#array-of-function-binding) | Array of function bindings (ScriptEvent). |
| `1A` | [Struct](#struct) | Struct field. | **Optional_1** |
| `1B` | Uuid | Uuid |
| `1C` | [Object Store](#object-store) | Array of attached objects |

## **Serialized Data**

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 8B | `xx xx xx xx xx xx xx xx` | Size of this table in bytes (including this field but including **End of data** field) |
| +08 | 4B | N | Number of entries in this list. i.e. number of all serialized objects. |
| | | | |
| | | | |
|  |  | **<-- Entry #0 -->** |  |
| +0C | 8B | `xx xx xx xx xx xx xx xx` | Size of 1-st entry in bytes. (including this field) (>= 8) |
| +10 | 4B | `xx xx xx xx` | Data start offset **after** this field. |
| +14 | 16B | 128 bit Uuid | Object Instance UUID. |
| +20 | 1B | `00/01` | Is Asset? `0` or `1` |
| +21 | 4B | Index into [Schema Table](#schema-table) | Index of this class layout in Schema Table. |
| +25 | \0 | `MySkybox.Irradiance.Diffuse\0` | Path to this object within the bundle |
| +xx | \0 | `MyObjectName\0` | Object name (CE::Name) |
| - | - | New header fields can be added here | - |
| - | - | Data starts from below | - |
| +xx | 8B | `00 00 00 00 00 00 00 08` | Size of ALL the fields in bytes including this. (>= 8) |
|  |  | Field #0 | Number of fields can be inferred from Schema Table. |
| +08 | xx | [Field Value](#field-value) | [Field Value](#field-value) depending on the [Field Type](#field-type) found in Schema Table. |
|  |  | Field #1 | Number of fields can be inferred from Schema Table. |
| +08 | xx | [Field Value](#field-value) | [Field Value](#field-value) depending on the [Field Type](#field-type) found in Schema Table. |
| | | | |
| | | | |
| | 4B | `00 00 00 00` | **End of data** |

## **Field Value**

### String data type
| Field Type | Size | Format | Desc |
|---|---|---|---|
| String | `\0` | `Null Terminated String\0` | `0` is valid. |

### Binary data type
| Field Type | Size | Format | Desc |
|---|---|---|---|
| Byte Size | 8B | `xx xx xx xx xx xx xx xx` | `0` is valid size. |
| Flags | 8B | `xx xx xx xx xx xx xx xx` | BinaryBlob flags value. |
| xx | xx |  | Raw binary data. |

### Object Reference

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 16B | 128 bit Uuid | Object UUID. |
| +10 | 16B | 128 bit Uuid | Bundle UUID the referenced object belongs to. |

### NULL Object Reference

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 16B | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` | 0 Uuid |

### Object Store

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 4B | `00 00 00 02` | Number of sub-objects.  |
| +04 | 16B | 128 bit Uuid | 1st Object UUID. |
| +14 | 16B | 128 bit Uuid | 1st Bundle UUID the referenced object belongs to. |
| +24 | 16B | 128 bit Uuid | 2nd Object UUID. |
| +34 | 16B | 128 bit Uuid | 2nd Bundle UUID the referenced object belongs to. |

### Function Binding

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 16B | 128 bit Uuid | UUID of the bound **object**. |
| +10 | 16B | 128 bit Uuid | UUID of the **bundle** the bound object is in. |
| +20 | \0 | `FunctionName\0` | Exact name of function as in C++. |

### NULL Function Binding

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 16B | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` | 0 Uuid. |

### Array of Function Binding

| Offset | Size | Value | Description |
|---|---|---|---|
| +xx | 4B | `00 00 00 02` | Number of bindings.  |
| +xx | 16B | 128 bit Uuid | 1st. Object UUID. |
| +xx | 16B | 128 bit Uuid | 1st. Bundle UUID the referenced object belongs to. |
| +xx | \0 | `FunctionName\0` | 1st. Exact name of function as in C++. |
| +xx | 16B | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` | 2nd. NULL binding. |
| +xx | 16B | 128 bit Uuid | 3rd. Object UUID. |
| +xx | 16B | 128 bit Uuid | 3rd. Bundle UUID the referenced object belongs to. |
| +xx | \0 | `FunctionName\0` | 3rd. Exact name of function as in C++. |

### Struct

| Offset | Size | Value | Description |
|---|---|---|---|
| +xx | xx | [Field Value](#field-value) | Value of 1st field |
| +xx | xx | [Field Value](#field-value) | Value of 2nd field |

### Array

| Offset | Size | Value | Description |
|---|---|---|---|
| +00 | 4B | `00 00 00 02` | Number of elements in array  |
| +04 | xx | [Field Value](#field-value) | Value of 1st element |
| +xx | xx | [Field Value](#field-value) | Value of 2nd element |

## Field Serializer

Object, struct fields and array elements have the same format:

| Offset | Size | Value | Description |
|---|---|---|---|
| +08 | xx | [Field Value](#field-value) | Value of field |

