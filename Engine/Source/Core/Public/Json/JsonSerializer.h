#pragma once

#include "JsonReader.h"
#include "JsonWriter.h"

namespace CE
{
    class CORE_API JsonSerializer
    {
    public:
        CE_STATIC_CLASS(JsonSerializer);

		CE_DEPRECATED("Use Deserialize2 instead.") static JsonValue* Deserialize(Stream* stream);

		static bool Deserialize2(Stream* stream, JValue& out);

		static bool Deserialize2(const String& json, JValue& out);
		
        template<typename WritePolicy = JsonPrettyPrintPolicy>
		CE_DEPRECATED("Use Serialize2 instead.") static void Serialize(Stream* stream, const JsonValue* json)
        {
            if (stream == nullptr || !stream->CanWrite())
                return;
            if (json == nullptr || !json->IsContainerType())
                return;

			JsonWriter<WritePolicy> writer = JsonWriter<WritePolicy>::Create(stream);
            bool isObject = json->IsObjectValue();

            if (json->IsObjectValue())
            {
                writer.WriteObjectStart();
                {
                    SerializeInternal(writer, json);
                }
                writer.WriteObjectClose();
            }
            else if (json->IsArrayValue())
            {
                writer.WriteArrayStart();
                {
                    SerializeInternal(writer, json);
                }
                writer.WriteArrayClose();
            }

			if (stream->IsBinaryMode())
				stream->Write('\0');
        }

		template<typename WritePolicy = JsonPrettyPrintPolicy>
		static void Serialize2(Stream* stream, const JValue& json)
		{
			if (stream == nullptr || !stream->CanWrite() || !stream->IsAsciiMode())
			{
				CE_LOG(Error, All, "Invalid stream! Stream must be writeable and in ascii mode");
				return;
			}
			if (json.IsNullValue() || !json.IsContainerType())
				return;

			auto writer = JsonWriter<WritePolicy>::Create(stream);
			const bool isObject = json.IsObjectValue();

			if (json.IsObjectValue())
			{
				writer.WriteObjectStart();
				{
					SerializeInternal2(writer, json);
				}
				writer.WriteObjectClose();
			}
			else if (json.IsArrayValue())
			{
				writer.WriteArrayStart();
				{
					SerializeInternal2(writer, json);
				}
				writer.WriteArrayClose();
			}

			if (stream->IsBinaryMode())
				stream->Write('\0');
		}
        
    private:

        template<typename WritePolicy = JsonPrettyPrintPolicy>
        static void SerializeInternal(JsonWriter<WritePolicy>& writer, const JsonValue* parent)
        {
            if (parent == nullptr || !parent->IsContainerType())
                return;

            if (parent->IsObjectValue())
            {
                for (const auto& [identifier, json] : parent->objectValue)
                {
                    if (json->IsObjectValue())
                    {
                        writer.WriteObjectStart(identifier);
                        {
                            SerializeInternal(writer, json);
                        }
                        writer.WriteObjectClose();
                    }
                    else if (json->IsArrayValue())
                    {
                        writer.WriteArrayStart(identifier);
                        {
                            SerializeInternal(writer, json);
                        }
                        writer.WriteArrayClose();
                    }
                    else if (json->IsStringValue())
                    {
                        writer.WriteValue(identifier, json->stringValue);
                    }
                    else if (json->IsBoolValue())
                    {
                        writer.WriteValue(identifier, json->boolValue);
                    }
                    else if (json->IsNumberValue())
                    {
                        writer.WriteValue(identifier, json->numberValue);
                    }
                    else if (json->IsNullValue())
                    {
                        writer.WriteNull(identifier);
                    }
                }
            }
            else if (parent->IsArrayValue())
            {
                for (const auto& json : parent->arrayValue)
                {
                    if (json->IsObjectValue())
                    {
                        writer.WriteObjectStart();
                        {
                            SerializeInternal(writer, json);
                        }
                        writer.WriteObjectClose();
                    }
                    else if (json->IsArrayValue())
                    {
                        writer.WriteArrayStart();
                        {
                            SerializeInternal(writer, json);
                        }
                        writer.WriteArrayClose();
                    }
                    else if (json->IsStringValue())
                    {
                        writer.WriteValue(json->stringValue);
                    }
                    else if (json->IsBoolValue())
                    {
                        writer.WriteValue(json->boolValue);
                    }
                    else if (json->IsNumberValue())
                    {
                        writer.WriteValue(json->numberValue);
                    }
                    else if (json->IsNullValue())
                    {
                        writer.WriteNull();
                    }
                }
            }
        }

		template<typename WritePolicy = JsonPrettyPrintPolicy>
		static void SerializeInternal2(JsonWriter<WritePolicy>& writer, const JValue& parent)
		{
			if (parent.IsNullValue() || !parent.IsContainerType())
				return;

			if (parent.IsObjectValue())
			{
				for (const auto& [identifier, json] : parent.objectValue)
				{
					if (json.IsObjectValue())
					{
						writer.WriteObjectStart(identifier);
						{
							SerializeInternal2(writer, json);
						}
						writer.WriteObjectClose();
					}
					else if (json.IsArrayValue())
					{
						writer.WriteArrayStart(identifier);
						{
							SerializeInternal2(writer, json);
						}
						writer.WriteArrayClose();
					}
					else if (json.IsStringValue())
					{
						writer.WriteValue(identifier, json.stringValue);
					}
					else if (json.IsBoolValue())
					{
						writer.WriteValue(identifier, json.boolValue);
					}
					else if (json.IsNumberValue())
					{
						writer.WriteValue(identifier, json.numberValue);
					}
					else if (json.IsNullValue())
					{
						writer.WriteNull(identifier);
					}
				}
			}
			else if (parent.IsArrayValue())
			{
				for (const auto& json : parent.arrayValue)
				{
					if (json.IsObjectValue())
					{
						writer.WriteObjectStart();
						{
							SerializeInternal2(writer, json);
						}
						writer.WriteObjectClose();
					}
					else if (json.IsArrayValue())
					{
						writer.WriteArrayStart();
						{
							SerializeInternal2(writer, json);
						}
						writer.WriteArrayClose();
					}
					else if (json.IsStringValue())
					{
						writer.WriteValue(json.stringValue);
					}
					else if (json.IsBoolValue())
					{
						writer.WriteValue(json.boolValue);
					}
					else if (json.IsNumberValue())
					{
						writer.WriteValue(json.numberValue);
					}
					else if (json.IsNullValue())
					{
						writer.WriteNull();
					}
				}
			}
		}
        
    };
    
} // namespace CE
