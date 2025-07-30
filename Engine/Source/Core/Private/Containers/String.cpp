
#include <regex>

#include "CoreMinimal.h"

#include "NaturalLess.h"
#include "Object/Bundle.h"

using namespace CE;

namespace CE
{

    String::String() : String("")
    {
        
    }

    String::String(std::string string) : String(string.c_str())
    {

    }

    String::String(StringView stringView) : String()
    {
        Reserve(stringView.GetSize());
        CopyCString(stringView.GetCString(), stringView.GetSize());
    }

    String::String(u32 reservedSize)
    {
        Reserve(reservedSize);
    }

    String::String(const char* value)
    {
        if (value == nullptr)
        {
            Reserve(1);
            SetCString("");
            return;
        }

        Reserve(strlen(value));
        SetCString(value);
    }

    String::String(const char* cStringA, const char* cStringB)
    {
        Reserve(strlen(cStringA) + strlen(cStringB));
        SetCString(cStringA);
        ConcatenateCString(cStringB);
    }

    String::String(Iterator begin, Iterator end)
    {
        auto length = end.Ptr - begin.Ptr;
        Reserve(length);
        CopyCString(begin.Ptr, length);
    }

    String::String(String&& move) noexcept
        : Buffer(move.Buffer)
        , Capacity(move.Capacity)
        , StringLength(move.StringLength)
    {
        // Zero out 'move'
        move.Buffer = nullptr;
        move.Capacity = 0;
        move.StringLength = 0;
    }

    String::String(const String& copy)
    {
        CopyCString(copy.GetCString(), copy.StringLength);
    }

    String& String::operator=(const String& rhs)
    {
        if (&rhs != this)
        {
            CopyCString(rhs.GetCString(), rhs.StringLength);
        }
        return *this;
    }

    String& String::operator=(const std::string& rhs)
    {
        Reserve(rhs.length());
        SetCString(rhs.c_str());
        return *this;
    }

    String& String::operator=(const char* cString)
    {
        SetCString(cString);
        return *this;
    }

    String::~String()
	{
        {
            delete[] Buffer;
        }

        Buffer = nullptr;
        Capacity = StringLength = 0;
    }

    void String::Reserve(u32 reserveCharacterCount)
    {
        // High capacity values are often because of garbage values and can cause crashes because String buffer is pointing to garbage location
        if (Capacity > 16_MB)
        {
            // Reset string to defaults
            Capacity = 0;
            Buffer = nullptr;
			return;
        }

        reserveCharacterCount++; // Add extra byte for null terminator

        //if (reserveCharacterCount > STRING_BUFFER_SIZE) // Use dynamic buffer
        {
            if (Buffer == nullptr) // If we need a dynamic buffer from the get-go
            {
                Capacity = reserveCharacterCount;
                Buffer = new char[Capacity];
                Buffer[0] = 0;
            }
            //else if (impl != nullptr && Buffer != nullptr) // If we were using static buffer earlier
            //{
            //    auto staticBuffer = Buffer;
            //    Capacity = reserveCharacterCount;
            //    Buffer = new char[Capacity];
            //    if (StringLength > 0)
            //        memcpy(Buffer, staticBuffer, StringLength + 1);
            //    else
            //        Buffer[0] = 0;
            //    ((Internal::StaticStringBlockAllocator*)impl)->Free(staticBuffer);
            //    impl = nullptr;
            //}
            else if (reserveCharacterCount > Capacity && Buffer != nullptr) // If we were already using dynamic buffer, but need more capacity
            {
                auto stagingBuffer = new char[reserveCharacterCount];
                if (StringLength > 0)
                    memcpy(stagingBuffer, Buffer, StringLength + 1);
                else
                    memset(stagingBuffer, 0, reserveCharacterCount);

                Capacity = reserveCharacterCount;
                delete[] Buffer;
                Buffer = stagingBuffer;
            }
        }
  //      else if (Buffer == nullptr) // If static buffer is enough, and it is not yet created
  //      {
  //          impl = &Internal::StaticStringBlockAllocator::Get();
  //          Capacity = reserveCharacterCount;
  //          Buffer = (char*)((Internal::StaticStringBlockAllocator*)impl)->Allocate();
  //          Buffer[0] = 0;
  //      }
		//else if (impl != nullptr && Buffer != nullptr && reserveCharacterCount > Capacity) // If static buffer is enough, but we need more capacity
		//{
		//	Capacity = reserveCharacterCount;
		//}
    }

    void String::Free()
    {
		//if (impl != nullptr && Buffer != nullptr)
		//{
  //          ((Internal::StaticStringBlockAllocator*)impl)->Free(Buffer);
		//}
		//else
		{
			delete[] Buffer;
		}

		Buffer = nullptr;
		Capacity = StringLength = 0;
        //impl = nullptr;
    }

    char* String::GetCString() const
    {
        return Buffer;
    }

	char* String::GetData() const
	{
		return Buffer;
	}

    bool String::IsEmptyOrWhiteSpace() const
    {
        if (IsEmpty())
			return true;

        for (int i = 0; i < StringLength; ++i)
        {
            if (Buffer[i] != ' ')
                return false;
        }

        return true;
    }

    bool String::NaturalCompare(const String& lhs, const String& rhs)
    {
		auto left = lhs.ToLower();
		auto right = rhs.ToLower();
		int result = strcmp_natural(left.GetCString(), right.GetCString());
		return result < 0;
    }

	bool String::Compare(const String& lhs, const String& rhs)
	{
		if (lhs.GetLength() < rhs.GetLength())
			return true;
		else if (lhs.GetLength() > rhs.GetLength())
			return true;

		for (int i = 0; i < lhs.GetLength(); i++)
		{
			char l = lhs[i];
			char r = rhs[i];
			if (IsAlphabet(l))
				l = std::tolower(l);
			if (IsAlphabet(r))
				r = std::tolower(r);

			if (l < r)
				return true;
			else if (l > r)
				return false;
		}

		return false;
	}

    StringView CE::String::ToStringView() const
    {
        return StringView(*this);
    }

    void String::SetCString(const char* cString)
    {
        if (cString == nullptr) // Clear the string
        {
            if (Capacity > 0) Buffer[0] = 0;
            StringLength = 0;
            return;
        }

        StringLength = strlen(cString);

        if (StringLength == 0) // Clear the string
        {
            if (Capacity > 0) Buffer[0] = 0;
            return;
        }

        Reserve(StringLength);
        memcpy(Buffer, cString, StringLength + 1);
        Buffer[StringLength] = 0;
    }

    void String::CopyCString(const char* cString, u32 copyStringLength)
    {
        Reserve(copyStringLength); // reserve: copyStringLength + 1

        if (cString == nullptr) // Clear the string
        {
            if (Capacity > 0 && Buffer != nullptr) Buffer[0] = 0;
            return;
        }

        StringLength = copyStringLength;

        if (StringLength == 0) // Clear the string
        {
            if (Capacity > 0 && Buffer != nullptr) 
                Buffer[0] = 0;
            return;
        }

        Reserve(StringLength);
		if (Buffer != nullptr)
		{
			memcpy(Buffer, cString, StringLength);
			Buffer[StringLength] = 0;
		}
    }

    /*
     *  Helper/Utility
     */

    bool String::RegexMatch(const String& sourceString, const String& regex)
    {
        return std::regex_match(sourceString.GetCString(), std::regex(regex.GetCString()));
    }

    bool String::RegexMatch(const String& sourceString, const char* regex)
    {
        return std::regex_match(sourceString.GetCString(), std::regex(regex));
    }

    void String::Append(char c)
    {
        char copyStr[] = { c, '\0' };
        ConcatenateCString(copyStr);
    }

    void String::Concatenate(const String& string)
    {
        ConcatenateCString(string.GetCString());
    }

    void String::ConcatenateCString(const char* cString)
    {
        auto cStringLength = strlen(cString);
        Reserve(StringLength + cStringLength);
        memcpy(Buffer + StringLength, cString, cStringLength + 1);
        StringLength += cStringLength;
    }

    void String::Concatenate(s64 integer)
    {
        Concatenate(std::to_string(integer));
    }

    bool String::StartsWith(const String& string) const
    {
        return StartsWith(string.GetCString());
    }

    bool String::StartsWith(const char* cString) const
    {
        int charIndex = 0;
        if (cString == nullptr)
            return true;

        while (*cString != 0)
        {
            if (charIndex >= GetLength())
                return false;

            if (Buffer[charIndex] != *cString)
                return false;

            charIndex++;
            cString++;
        }

        return true;
    }

    bool String::EndsWith(const String& string) const
    {
        int thisIdx = GetLength() - 1;
        int otherIdx = string.GetLength() - 1;

        if (GetLength() < string.GetLength())
            return false;

        while (thisIdx >= 0 && otherIdx >= 0)
        {
            if (Buffer[thisIdx] != string[otherIdx])
                return false;

            thisIdx--;
            otherIdx--;
        }

        return true;
    }

    bool String::EndsWith(const char* cString) const
    {
        return EndsWith(String(cString));
    }

    bool String::Contains(const String& string) const
    {
        int thisIndex = 0;

        while (thisIndex < GetLength())
        {
            StringView stringView = StringView(Buffer + thisIndex, GetLength() - thisIndex);
            if (stringView.StartsWith(string.ToStringView()))
            {
                return true;
            }

            thisIndex++;
        }

        return false;
    }

    bool CE::String::Contains(const char* string) const
    {
        int thisIndex = 0;
        StringView other = StringView(string);

        while (thisIndex < GetLength())
        {
            StringView stringView = StringView(Buffer + thisIndex, GetLength() - thisIndex);
            if (stringView.StartsWith(other))
            {
                return true;
            }

            thisIndex++;
        }

        return false;
    }

    bool String::Contains(char character) const
    {
        for (int i = 0; i < GetLength(); i++)
        {
            if (Buffer[i] == character)
                return true;
        }

        return false;
    }

    bool CE::String::Search(const String& string) const
    {
        int thisIndex = 0;
        String thisLower = ToLower();
        String otherLower = string.ToLower();

        return thisLower.Contains(otherLower);
    }

    bool CE::String::Search(const char* string) const
    {
        int thisIndex = 0;
        String thisLower = ToLower();
        String otherLower = String(string).ToLower();

        return thisLower.Contains(otherLower);
    }

    String CE::String::ToLower() const
    {
        String result{ GetLength() };

        for (int i = 0; i < GetLength(); i++)
        {
            char ch = Buffer[i];
            if (ch >= 'A' && ch <= 'Z')
                result[i] = std::tolower(ch);
            else
                result[i] = ch;
        }

        result.Buffer[GetLength()] = 0;
        result.StringLength = std::strlen(result.Buffer);

        return result;
    }

    String CE::String::ToUpper() const
    {
        String result{ GetLength() };

        for (int i = 0; i < GetLength(); i++)
        {
            char ch = Buffer[i];
            if (ch >= 'a' && ch <= 'z')
                result[i] = std::toupper(ch);
            else
                result[i] = ch;
        }

        result.Buffer[GetLength()] = 0;
        result.StringLength = std::strlen(result.Buffer);

        return result;
    }

	String String::ToKebabCase() const
	{
		String result{};

		for (int i = 0; i < GetLength(); i++)
		{
			if (i == 0)
			{
				if (String::IsAlphabet(Buffer[i]))
					result.Append((char)std::tolower(Buffer[i]));
				else
					result.Append(Buffer[i]);
				continue;
			}
			if (Buffer[i] == '_')
			{
				continue;
			}

			if (String::IsAlphabet(Buffer[i]) && (Buffer[i - 1] == '_' || std::isupper(Buffer[i])))
			{
				result.Append('-');
				result.Append((char)std::tolower(Buffer[i]));
				continue;
			}

			result.Append(Buffer[i]);
		}

		return result;
	}

	String String::ToSnakeCase() const
	{
		String result{};

		for (int i = 0; i < GetLength(); i++)
		{
			if (i == 0)
			{
				if (String::IsAlphabet(Buffer[i]))
					result.Append((char)std::tolower(Buffer[i]));
				else
					result.Append(Buffer[i]);
				continue;
			}
			if (Buffer[i] == '-')
			{
				continue;
			}

			if (String::IsAlphabet(Buffer[i]) && (Buffer[i - 1] == '-' || std::isupper(Buffer[i])))
			{
				result.Append('_');
				result.Append((char)std::tolower(Buffer[i]));
				continue;
			}
			
			result.Append(Buffer[i]);
		}

		return result;
	}

	String String::ToCamelCase() const
	{
		String result{};

		for (int i = 0; i < GetLength(); i++)
		{
			if (i == 0)
			{
				if (String::IsAlphabet(Buffer[i]))
					result.Append((char)std::tolower(Buffer[i]));
				else
					result.Append(Buffer[i]);
				continue;
			}
			if (Buffer[i] == '-' || Buffer[i] == '_')
			{
				continue;
			}

			if (String::IsAlphabet(Buffer[i]) && (Buffer[i - 1] == '-' || Buffer[i - 1] == '_' || std::isupper(Buffer[i])))
			{
				result.Append((char)std::toupper(Buffer[i]));
				continue;
			}

			result.Append(Buffer[i]);
		}

		return result;
	}

	String String::ToPascalCase() const
	{
		String result{};

		for (int i = 0; i < GetLength(); i++)
		{
			if (i == 0)
			{
				if (String::IsAlphabet(Buffer[i]))
					result.Append((char)std::toupper(Buffer[i]));
				else
					result.Append(Buffer[i]);
				continue;
			}
			if (Buffer[i] == '-' || Buffer[i] == '_')
			{
				continue;
			}

			if (String::IsAlphabet(Buffer[i]) && (Buffer[i - 1] == '-' || Buffer[i - 1] == '_' || std::isupper(Buffer[i])))
			{
				result.Append((char)std::toupper(Buffer[i]));
				continue;
			}

			result.Append(Buffer[i]);
		}

		return result;
	}

    String String::GetSubstring(int startIndex, int length) const
    {
        if (length == -1)
        {
            return String(Buffer + startIndex);
        }

        if (length == 0)
        {
            return "";
        }

        return String(Iterator(cbegin().Ptr + startIndex), Iterator(cbegin().Ptr + startIndex + length));
    }

    StringView CE::String::GetSubstringView(int startIndex, int length) const
    {
        if (length == -1)
        {
            return StringView(Buffer + startIndex);
        }

        if (length == 0)
        {
            return StringView();
        }

        return StringView(GetCString() + startIndex, length);
    }

    Array<String> String::Split(char delimiter) const
    {
        int startIdx = 0;
        int endIdx = 0;

        Array<String> result{};

        for (int i = 0; i < StringLength; i++)
        {
            if (Buffer[i] == delimiter && startIdx < StringLength)
            {
				if (startIdx < endIdx)
					result.Add(GetSubstringView(startIdx, endIdx - startIdx)); // Don't add +1: we don't want the delimiter present in the split string

                startIdx = i + 1;
                endIdx = startIdx;
                continue;
            }
            else if (i == StringLength - 1) // last character
            {
                result.Add(GetSubstringView(startIdx, endIdx - startIdx + 1)); // +1 to include the last character
                continue;
            }

            endIdx++;
        }

        return result;
    }

    void CE::String::Split(String delimiter, Array<String>& outArray) const
    {
        int startIdx = 0;

        for (int i = 0; i < StringLength; i++)
        {
            char ch = Buffer[i];

            StringView view = StringView(Buffer + i);

            bool isLast = (i == StringLength - 1);
            if (view.StartsWith(delimiter) || isLast)
            {
                if (isLast)
                    i++;

                if (startIdx != i)
                {
                    outArray.Add(GetSubstringView(startIdx, i - startIdx));
                }

                startIdx = i + delimiter.GetLength();
                i += delimiter.GetLength() - 1;
            }
        }
    }

	void String::Split(const Array<String>& delimiters, Array<String>& outArray) const
	{
		int startIdx = 0;

		for (int i = 0; i < StringLength; i++)
		{
			char ch = Buffer[i];

			StringView view = StringView(Buffer + i);

			const bool isLast = (i == StringLength - 1);

            if (isLast)
            {
                bool foundDelimiter = false;

                for (const auto& delimiter : delimiters)
                {
                    if (view.StartsWith(delimiter))
                    {
                        if (startIdx != i)
                        {
                            outArray.Add(GetSubstringView(startIdx, i - startIdx));
                        }

                        startIdx = i + delimiter.GetLength();
                        i += delimiter.GetLength() - 1;
                        foundDelimiter = true;
                        break;
                    }
                }

                if (!foundDelimiter)
                {
                    i++;

                    if (startIdx != i)
                    {
                        outArray.Add(GetSubstringView(startIdx, i - startIdx));
                    }

                    break;
                }
            }
            else
            {
                for (const auto& delimiter : delimiters)
                {
                    if (view.StartsWith(delimiter) || isLast)
                    {
                        if (isLast)
                            i++;

                        if (startIdx != i)
                        {
                            outArray.Add(GetSubstringView(startIdx, i - startIdx));
                        }

                        startIdx = i + delimiter.GetLength();
                        i += delimiter.GetLength() - 1;
                        break;
                    }
                }
            }
		}
	}

	void String::Split(InitializerList<String> delimiters, Array<String>& outArray) const
	{
		Array<String> delims = delimiters;
		Split(delims, outArray);
	}

    String String::RemoveWhitespaces() const
    {
        char* result = new char[GetLength() + 1];
        result[GetLength()] = 0;
        bool isString = false;
        int idx = 0;

        for (int i = 0; i < GetLength(); i++)
        {
            if (Buffer[i] == '"')
            {
                isString = !isString;
                continue;
            }

            if (Buffer[i] == ' ' && !isString)
            {
                continue;
            }

            result[idx++] = Buffer[i];
        }

        result[idx++] = 0;

        auto str = String(result);
        delete result;
        return str;
    }

	String String::Replace(const Array<char>& charsToReplace, char replaceWith) const
	{
		if (replaceWith == 0 || charsToReplace.IsEmpty())
			return *this;

		String result = String();
		
		for (int i = 0; i < GetLength(); i++)
		{
			if (charsToReplace.Exists(Buffer[i]))
			{
				result.Append(replaceWith);
				continue;
			}

			result.Append(Buffer[i]);
		}

		return result;
	}

	void String::RemoveAt(int index)
	{
        if (index < 0 || index >= StringLength || StringLength < 1)
            return;

        for (int i = index; i < StringLength - 1; ++i)
        {
            Buffer[i] = Buffer[i + 1];
        }

        Buffer[StringLength - 1] = 0;
        StringLength--;
	}

	void String::InsertAt(char c, int index)
	{
        Reserve(GetLength() + 1);

        StringLength++;

        for (int i = StringLength - 1; i >= index; --i)
        {
            if (i > 0)
            {
	            Buffer[i] = Buffer[i - 1];
            }
        }

        Buffer[index] = c;
        Buffer[StringLength] = 0;
	}

    void String::Remove(int startIndex, int count)
    {
        if (count < 0)
        {
	        count = (int)StringLength - startIndex;
        }
        if (startIndex + count > StringLength)
        {
            count = StringLength - startIndex;
        }

        for (int i = startIndex; i < StringLength; ++i)
        {
            char value = Buffer[Math::Min<int>(count + i, StringLength)];

            Buffer[i] = value;

            if (value == 0)
                break;
        }

        StringLength -= count;
        Buffer[StringLength] = 0;
    }

	void String::UpdateLength()
	{
		StringLength = (u32)std::strlen(Buffer);
	}

	bool String::TryParse(const String& string, c8& outValue)
	{
		s64 value = 0;
		if (TryParseInteger(string, value))
		{
			outValue = static_cast<c8>(value);
			return true;
		}
		return false;
	}

	bool String::TryParse(const String& string, c16& outValue)
	{
		s64 value = 0;
		if (TryParseInteger(string, value))
		{
			outValue = static_cast<c16>(value);
			return true;
		}
		return false;
	}

	bool String::TryParse(const String& string, u8& outValue)
    {
        s64 value = 0;
        if (TryParseInteger(string, value))
        {
            outValue = static_cast<u8>(value);
            return true;
        }
        return false;
    }

    bool String::TryParse(const String& string, s8& outValue)
    {
        s64 value = 0;
        if (TryParseInteger(string, value))
        {
            outValue = static_cast<s8>(value);
            return true;
        }
        return false;
    }

	bool String::TryParse(const String& string, u16& outValue)
	{
		s64 value = 0;
		if (TryParseInteger(string, value))
		{
			outValue = static_cast<u16>(value);
			return true;
		}
		return false;
	}

	bool String::TryParse(const String& string, s16& outValue)
	{
		s64 value = 0;
		if (TryParseInteger(string, value))
		{
			outValue = static_cast<s16>(value);
			return true;
		}
		return false;
	}

    bool String::TryParse(const String& string, u32& outValue)
    {
        s64 value = 0;
        if (TryParseInteger(string, value))
        {
            outValue = static_cast<u32>(value);
            return true;
        }
        return false;
    }

    bool String::TryParse(const String& string, s32& outValue)
    {
        s64 value = 0;
        if (TryParseInteger(string, value))
        {
            outValue = static_cast<s32>(value);
            return true;
        }
        return false;
    }

    bool String::TryParse(const String& string, u64& outValue)
    {
        s64 value = 0;
        if (TryParseInteger(string, value))
        {
            outValue = static_cast<u64>(value);
            return true;
        }
        return false;
    }

    bool String::TryParse(const String& string, s64& outValue)
    {
        s64 value = 0;
        if (TryParseInteger(string, value))
        {
            outValue = static_cast<s64>(value);
            return true;
        }
        return false;
    }

    bool String::TryParse(const String& string, f32& outValue)
    {
        f32 value = 0;
        if (TryParseFloat(string, value))
        {
            outValue = value;
            return true;
        }
        return false;
    }

    bool String::TryParse(const String& string, f64& outValue)
    {
        f64 value = 0;
        if (TryParseFloat(string, value))
        {
            outValue = value;
            return true;
        }
        return false;
    }

    bool String::TryParse(const String& string, b8& outValue)
    {
        String lower = string.ToLower();
        if (lower == "true")
        {
            outValue = true;
            return true;
        }
        else if (lower == "false")
        {
            outValue = false;
            return true;
        }

        s64 value = 0;
        if (TryParseInteger(string, value))
        {
            outValue = value > 0;
            return true;
        }

        return false;
    }

    void String::Internal_ThrowParseException(const char* message)
    {
        throw ParseFailedException(message);
    }

    bool String::TryParseInteger(String value, s64& outValue)
    {
        if (value.IsEmpty())
            return false;
        
        value = value.RemoveWhitespaces();
        
        if (value.Contains('.') || value.Contains('f'))
            return false;

        s64 result = 0;

        int length = value.GetLength();
        bool isNegative = false;

        for (int i = length - 1; i >= 0; i--)
        {
            if (i > 0)
            {
                if (!IsNumeric(value[i]))
                    return false;
            }
            else
            {
                if (!IsNumeric(value[i]) && value[i] != '+' && value[i] != '-')
                    return false;
                if (value[i] == '+' || value[i] == '-')
                {
                    isNegative = value[i] == '-';
                    continue;
                }
            }
            
            result += Math::Pow((s64)10, length - 1 - i) * CharToNumber(value[i]);
        }

        if (isNegative)
            result *= -1;

        outValue = result;
        
        return true;
    }

    bool String::TryParseFloat(String value, f32& outValue)
    {
        if (value.IsEmpty())
            return false;
        
        value = value.RemoveWhitespaces();

        try
        {
            outValue = std::stof(value.ToStdString());
        }
        catch (...)
        {
            return false;
        }
        
        return true;
    }

    bool String::TryParseFloat(String value, f64& outValue)
    {
        if (value.IsEmpty())
            return false;
        
        value = value.RemoveWhitespaces();
        
        try
        {
            outValue = std::stod(value.ToStdString());
        }
        catch (...)
        {
            return false;
        }
        
        return true;
    }
}


