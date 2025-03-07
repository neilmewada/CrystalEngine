#pragma once

namespace CE
{
    
    typedef Delegate<bool(const String&)> FTextInputValidator;

    template<typename T> requires TIsNumericType<T>::Value
    bool FNumericInputValidator(const String& value)
    {
        if (value.IsEmpty())
            return true;

        if constexpr (TIsSameType<T, f32>::Value || TIsSameType<T, f64>::Value)
        {
            int periodCount = 0;
            static constexpr char infinity[] = "inf";
            bool isInfinity = false;

	        for (int i = 0; i < value.GetLength(); ++i)
	        {
                if (value[i] == ' ')
                    return false;

		        if (i == 0 && value[0] != '+' && value[0] != '-' && 
                    !String::IsNumeric(value[0]) && value[0] != '.' && 
                    value[0] != 'i' && value[0] != 'I')
		        {
                    return false;
		        }

                if ((value[i] == '+' || value[i] == '-') && i > 0)
                {
                    return false;
                }

                if (i > 0 && (value[0] == '+' || value[0] == '-'))
                {
                    if (i - 1 < sizeof(infinity) - 1 &&
                        std::tolower(value[i]) == infinity[i - 1])
                    {
                        isInfinity = true;
	                    continue;
                    }
                }
                else
                {
	                if (i < sizeof(infinity) - 1 &&
                        std::tolower(value[i]) == infinity[i])
	                {
                        isInfinity = true;
		                continue;
	                }
                }

                if (isInfinity)
                {
                    return false;
                }

                if (value[i] == '.')
                {
                    periodCount++;
                    if (periodCount > 1)
                    {
	                    return false;
                    }
                }
                else if (i > 0 && !String::IsNumeric(value[i]))
                {
                    return false;
                }
	        }
        }
    	if constexpr (TContainsType<T, u8, u16, u32, u64, s8, s16, s32, s64>::Value)
        {
            constexpr bool isSigned = TContainsType<T, s8, s16, s32, s64>::Value;

            for (int i = 0; i < value.GetLength(); ++i)
            {
                if (value[i] == ' ' || value[i] == '.')
                    return false;

	            if (i == 0 && value[0] == '-' && !isSigned)
	            {
                    return false;
	            }

                if (i > 0 && (value[i] == '+' || value[i] == '-'))
                {
                    return false;
                }

                if (i > 0 && !String::IsNumeric(value[i]))
                {
                    return false;
                }
            }
        }

        return true;
    }

} // namespace CE
