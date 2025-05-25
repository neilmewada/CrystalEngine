
#include "CoreMinimal.h"

#include <cstring>

namespace CE
{
	static SharedMutex nameMutex{};
	HashMap<SIZE_T, String> Name::nameHashMap = HashMap<SIZE_T, String>{
		{ 0, "" }
	};

	Name::Name(String name)
	{
		String value = "";

		if (name.IsEmpty())
		{
			hashValue = 0;
			return;
		}

		int i = 0;
		int length = name.GetLength();

		while (i < length && (name[i] == ':' || name[i] == '.')) // Remove all ':' or '.' from prefix
		{
			i++;
		}

		while (length > i && (name[length - 1] == ':' || name[length - 1] == '.')) // Remove all ':' or '.' from suffix
		{
			length--;
		}

		if (i >= length)
		{
			value = "";
			hashValue = 0;
			return;
		}

		value = name.GetSubstringView(i, length);

		String hashString = "";

		i = 0;
		
		while (i < length)
		{
			if (i < length - 1 && value[i] == ':' && value[i + 1] == ':')
			{
				i += 2;
				continue;
			}
			else
			{
				int startIdx = i;
				int len = 0;

				while (i < length)
				{
					i++;
					len++;

					if (i < length - 1 && value[i] == ':' && value[i + 1] == ':')
					{
						i++;
						break;
					}
				}

				if (!hashString.IsEmpty())
					hashString += "::";
				hashString += String(value.GetSubstringView(startIdx, len));
			}

			i++;
		}

		this->hashValue = GetHash(hashString);

		LockGuard lock{ nameMutex };

		nameHashMap[this->hashValue] = hashString;
#if CE_NAME_DEBUG
		debugString = nameHashMap[this->hashValue].GetCString();
#endif
	}

	Name::Name(const char* name) : Name(String(name))
	{
		
	}

	Name::Name(const Name& copy)
	{
		this->hashValue = copy.hashValue;
#if CE_NAME_DEBUG
		LockGuard lock{ nameMutex };
		debugString = nameHashMap[this->hashValue].GetCString();
#endif
	}

	Name& Name::operator=(const Name& copy)
	{
		this->hashValue = copy.hashValue;
#if CE_NAME_DEBUG
		LockGuard lock{ nameMutex };
		debugString = nameHashMap[this->hashValue].GetCString();
#endif
		return *this;
	}

	Name::Name(Name&& move)
	{
		hashValue = move.hashValue;
		move.hashValue = 0;

#if CE_NAME_DEBUG
		LockGuard<SharedMutex> lock{ nameMutex };
		debugString = nameHashMap[this->hashValue].GetCString();
#endif
	}

	const String& Name::GetString() const
	{
		LockGuard lock{ nameMutex };
		return nameHashMap[hashValue];
	}

	void Name::GetComponents(CE::Array<String>& components) const
	{
		components.Clear();
		String value = GetString();
		value.Split({ "/", "::" }, components);
	}

	String Name::GetLastComponent() const
	{
		String last = "";
		int i = 0;
		String value = GetString();
		int length = value.GetLength();

		Array<String> split{};
		value.Split({ "/", "::" }, split);

		if (split.IsEmpty())
			return "";

		return split.GetLast();
	}

	String Name::GetParentPath() const
	{
		String value = GetString();
		int lastSlashIndex = -1;

		for (int i = 0; i < value.GetLength() - 1; ++i)
		{
			if (value[i] == '/')
			{
				lastSlashIndex = i;
			}
		}

		if (lastSlashIndex > 0)
		{
			return value.GetSubstring(0, lastSlashIndex);
		}

		return "";
	}
} // namespace CE

