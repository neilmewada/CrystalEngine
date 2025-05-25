
#include "Core.h"

#include "Include.h"

#pragma region Registration

CE_RTTI_STRUCT_IMPL(, VariantTests, VariantStruct)
CE_RTTI_CLASS_IMPL(, BundleTests, WritingTestObj1)
CE_RTTI_CLASS_IMPL(, BundleTests, WritingTestObj2)
CE_RTTI_CLASS_IMPL(, ObjectTests, BaseClass)
CE_RTTI_CLASS_IMPL(, ObjectTests, DerivedClassA)
CE_RTTI_CLASS_IMPL(, CDITests, AnotherObject)
CE_RTTI_CLASS_IMPL(, CDITests, BottomObject)
CE_RTTI_CLASS_IMPL(, CDITests, TestObject)
CE_RTTI_STRUCT_IMPL(, BundleTests, WritingTestStructBase)
CE_RTTI_STRUCT_IMPL(, BundleTests, WritingTestStruct1)
CE_RTTI_STRUCT_IMPL(, BundleTests, WritingTestStruct2)
CE_RTTI_STRUCT_IMPL(, CDITests, TestStruct)
CE_RTTI_STRUCT_IMPL(, JsonTests, InnerStruct)
CE_RTTI_STRUCT_IMPL(, JsonTests, SerializedData)

static void CERegisterModuleTypes()
{
    CE_REGISTER_TYPES(
		VariantTests::VariantStruct,
        BundleTests::WritingTestObj1,
		BundleTests::WritingTestObj2,
		BundleTests::MyTextureDescriptor,
		BundleTests::MyMaterial,
		BundleTests::MyMaterialProperty,
		BundleTests::MyScript,
		BundleTests::MyMesh,
		BundleTests::MyTexture,
		BundleTests::FilterMode,
        ObjectTests::BaseClass,
        ObjectTests::DerivedClassA,
        CDITests::AnotherObject,
		CDITests::BottomObject,
        CDITests::TestObject,
        BundleTests::WritingTestStructBase,
		BundleTests::WritingTestStruct1,
		BundleTests::WritingTestStruct2,
        CDITests::TestStruct,
        JsonTests::InnerStruct,
        JsonTests::SerializedData
    );
}
static void CEDeregisterModuleTypes()
{
    CE_DEREGISTER_TYPES(
		VariantTests::VariantStruct,
        BundleTests::WritingTestObj1,
        BundleTests::WritingTestObj2,
		BundleTests::MyMaterial,
		BundleTests::MyScript,
		BundleTests::MyMesh,
		BundleTests::MyTexture,
		BundleTests::MyTextureDescriptor,
		BundleTests::MyMaterialProperty,
		BundleTests::FilterMode,
        ObjectTests::BaseClass,
        ObjectTests::DerivedClassA,
        CDITests::AnotherObject,
		CDITests::BottomObject,
        CDITests::TestObject,
        BundleTests::WritingTestStructBase,
        BundleTests::WritingTestStruct1,
		BundleTests::WritingTestStruct2,
        CDITests::TestStruct,
        JsonTests::InnerStruct,
        JsonTests::SerializedData
    );
}

#pragma endregion

#include <iostream>
#include <any>

#include <gtest/gtest.h>

#define TEST_BEGIN\
    CE::gProjectPath = PlatformDirectories::GetLaunchDir();\
    CE::gProjectName = "Core_Test";\
    CE::ModuleManager::Get().LoadModule("Core");

#define TEST_END\
    CE::ModuleManager::Get().UnloadModule("Core");

#define LOG(x) std::cout << x << std::endl
#define LOG_ERR(x) std::cerr << x << std::endl;

using namespace CE;

/**********************************************
*   Performance
*/

#pragma region Performance

TEST(Performance, Module_Load_10x)
{
    TEST_BEGIN;
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().UnloadModule("Core");

    ModuleManager::Get().LoadModule("Core");
    TEST_END;
}

#pragma endregion


/**********************************************
*   Threading
*/

#pragma region Threading


static String ThreadingAsyncFunc(const Array<String>& array)
{
	String result = "";
	for (int i = 0; i < array.GetSize(); i++)
	{
		result += array[i];
		if (i != array.GetSize() - 1)
			result += ",";
	}
	Thread::SleepFor(20);
	return result;
}

class AsyncTestClass
{
public:
	AsyncTestClass()
	{

	}

	String ThreadingAsyncFunc(const Array<String>& array)
	{
		String result = "";
		for (int i = 0; i < array.GetSize(); i++)
		{
			result += array[i];
			if (i != array.GetSize() - 1)
				result += separator;
		}
		Thread::SleepFor(20);
		return result;
	}

	// This is NOT thread safe
	String separator = ",";
};

TEST(Threading, Async)
{
	TEST_BEGIN;

	// 1. Simple types
	{
		auto operation = [](const String& string) -> int
		{
			Thread::SleepFor(20);
			return string.GetLength();
		};

		int i = -1;

		auto future = Async(operation, String("123456789"));

		EXPECT_EQ(i, -1);
		i = future.Get();
		EXPECT_EQ(i, 9);
	}

	// 2. Monkey test
	{
		auto operation = [](const String& input) -> String
		{
			Thread::SleepFor(20);
			return input.Replace({ '_' }, '-');
		};

		String input = "this_is_a_string";
		
		auto future0 = Async(operation, input);

		EXPECT_EQ(input, "this_is_a_string");
		future0.Wait();
		future0.Wait();
		future0.Wait();
		future0.Wait();
		future0.Wait();
		future0.Wait();
		future0.Wait();
		future0.Wait();
		future0.Wait();
		future0.Wait();
		input = future0.Get();
		input = future0.Get();
		input = future0.Get();
		input = future0.Get();
		input = future0.Get();
		EXPECT_EQ(input, "this-is-a-string");
	}

	// 3. Global function with callback
	{
		Array<String> input = { "this", "is", "an", "array" };

		Delegate<void(const String&)> callback = [&](const String& value)
		{
			EXPECT_EQ(value, "this,is,an,array");
		};

		auto future = AsyncCallback(ThreadingAsyncFunc, callback, input);
		
		EXPECT_EQ(input.GetSize(), 4);
		EXPECT_EQ(input[0], "this");
		EXPECT_EQ(input[1], "is");
		EXPECT_EQ(input[2], "an");
		EXPECT_EQ(input[3], "array");

		// Wait for exec to complete, otherwise we will finish this test before results
		future.Wait();
	}

	TEST_END;
}

#pragma endregion


/**********************************************
*   Misc
*/

#pragma region Misc


#pragma endregion


/**********************************************
*   Containers
*/

#pragma region Containers

TEST(Containers, StringAllocation)
{
    String myString = "Hello World";

    auto t1 = Thread([&]
    {
        String t1String = "Inside Thread 1";
    });

    auto t2 = Thread([&]
    {
        myString = "New String";
    });

    if (t1.IsJoinable())
        t1.Join();
    if (t2.IsJoinable())
        t2.Join();

    EXPECT_EQ(myString, "New String");
}

TEST(Containers, String)
{
    TEST_BEGIN;

    // 1. Split tests

    String str = "CE::Editor::Qt::StyleManager";
    Array<String> components{};
    str.Split("::", components);
    EXPECT_EQ(components.GetSize(), 4);
    EXPECT_EQ(components[0], "CE");
    EXPECT_EQ(components[1], "Editor");
    EXPECT_EQ(components[2], "Qt");
    EXPECT_EQ(components[3], "StyleManager");
    
    str = "::::::CE::Editor";
    components.Clear();
    str.Split("::", components);
    EXPECT_EQ(components.GetSize(), 2);
    EXPECT_EQ(components[0], "CE");
    EXPECT_EQ(components[1], "Editor");

    str = "::";
    components.Clear();
    str.Split("::", components);
    EXPECT_EQ(components.GetSize(), 0);
    
    String str2 = "Obj1.Obj2.Obj3.";
    components.Clear();
    components = str2.Split('.');
    EXPECT_EQ(components.GetSize(), 3);
    EXPECT_EQ(components[0], "Obj1");
    EXPECT_EQ(components[1], "Obj2");
    EXPECT_EQ(components[2], "Obj3");

	str2.RemoveAt(4);
	str2.RemoveAt(8);
	EXPECT_EQ(str2, "Obj1Obj2Obj3.");

	str.RemoveAt(1);
	EXPECT_EQ(str, ":");

	String newStr = "";
	newStr.InsertAt('a', 0);
	newStr.InsertAt('c', 1);
	newStr.InsertAt('b', 1);

	EXPECT_EQ(newStr, "abc");

	String deleteString = "This is a long sentence string";
	deleteString.Remove(10, 5);
	EXPECT_EQ(deleteString, "This is a sentence string");
	deleteString.Remove(0, 5);
	EXPECT_EQ(deleteString, "is a sentence string");
	deleteString.Remove(2);
	EXPECT_EQ(deleteString, "is");

    {
		String empty = "";

		empty.InsertAt('T', 0);
    }

    // 2. Format Tests
    
    {
        EXPECT_EQ(String::Format("{1} - {0} - {2}", 12, "Str", 45.12f), "Str - 12 - 45.12");
        EXPECT_EQ(String::Format("Base{NAME} - {0}", 12, String::Arg("NAME", "Project")), "BaseProject - 12");
    }

    const String platformName = PlatformMisc::GetPlatformName();

    EXPECT_EQ(String::Format("{ENGINE}/Config/{PLATFORM}{TYPE}.ini",
        String::Arg("ENGINE", "@EnginePath"),
        String::Arg("PLATFORM", platformName),
        String::Arg("TYPE", "ProjectSettings")),
        "@EnginePath/Config/" + platformName + "ProjectSettings.ini");

    // 3. Parse Tests

	EXPECT_EQ(String::Parse<u16>("65000"), 65000);
	EXPECT_NE(String::Parse<u16>("65900"), 65000);
    EXPECT_EQ(String::Parse<s32>("1234567890"), 1234567890);
    EXPECT_EQ(String::Parse<s32>("-1234567890"), -1234567890);
    EXPECT_EQ(String::Parse<s32>("0001234"), 1234);
    EXPECT_EQ(String::Parse<s32>("-0001234"), -1234);
    
    EXPECT_EQ(String::Parse<f32>("12.242f"), 12.242f);
    EXPECT_EQ(String::Parse<f32>("12.242"), 12.242f);
    EXPECT_EQ(String::Parse<f32>("12"), 12);
    EXPECT_EQ(String::Parse<f32>("12f"), 12.f);
    EXPECT_EQ(String::Parse<f32>("12.f"), 12.f);

	// 4. Case Change Tests
    
	String camelCase = "thisIsAString";
	String pascalCase = "ThisIsAString";
	String kebabCase = "this-is-a-string";
	String snakeCase = "this_is_a_string";

	// camelCase
	EXPECT_EQ(camelCase.ToCamelCase(), camelCase);
	EXPECT_EQ(pascalCase.ToCamelCase(), camelCase);
	EXPECT_EQ(kebabCase.ToCamelCase(), camelCase);
	EXPECT_EQ(snakeCase.ToCamelCase(), camelCase);

	// PascalCase
	EXPECT_EQ(camelCase.ToPascalCase(), pascalCase);
	EXPECT_EQ(pascalCase.ToPascalCase(), pascalCase);
	EXPECT_EQ(kebabCase.ToPascalCase(), pascalCase);
	EXPECT_EQ(snakeCase.ToPascalCase(), pascalCase);

	// kebab-case
	EXPECT_EQ(camelCase.ToKebabCase(), kebabCase);
	EXPECT_EQ(pascalCase.ToKebabCase(), kebabCase);
	EXPECT_EQ(kebabCase.ToKebabCase(), kebabCase);
	EXPECT_EQ(snakeCase.ToKebabCase(), kebabCase);

	// snake_case
	EXPECT_EQ(camelCase.ToSnakeCase(), snakeCase);
	EXPECT_EQ(pascalCase.ToSnakeCase(), snakeCase);
	EXPECT_EQ(kebabCase.ToSnakeCase(), snakeCase);
	EXPECT_EQ(snakeCase.ToSnakeCase(), snakeCase);

    TEST_END;
}

TEST(Containers, Uuid)
{
	TEST_BEGIN;

	Uuid uuid = Uuid::Random();

	String uuidString = uuid.ToString();

	Uuid fromString = Uuid::FromString(uuidString);

	EXPECT_EQ(uuid, fromString);


	TEST_END;
}

TEST(Containers, Path)
{
	TEST_BEGIN;

	// Path splitting
	{
		IO::Path fullPath = "C:/Projects\\CrystalEngine/Engine\\Assets/Textures\\My_Tex.png";
		fullPath = fullPath.GetString().Replace({ '\\' }, '/');

		Array<String> components = {};
		fullPath.GetString().Split({ "/Engine/Assets" }, components);

		EXPECT_EQ(components.GetSize(), 2);
		EXPECT_EQ(components[0], "C:/Projects/CrystalEngine");
		EXPECT_EQ(components[1], "/Textures/My_Tex.png");
	}

	TEST_END;
}

TEST(Containers, Name)
{
    TEST_BEGIN;

    Name name = TYPENAME(Object);
    EXPECT_EQ(name, "/Code/Core.CE::Object");
    EXPECT_EQ(name.GetString(), "/Code/Core.CE::Object");
    EXPECT_EQ(name.GetLastComponent(), "Object");

    EXPECT_EQ(Name("::CE::Object"), Name("CE::Object"));
    EXPECT_EQ(Name("CE::Object::"), Name("CE::Object"));
    EXPECT_EQ(Name("::CE::Object"), Name("CE::Object::"));
    EXPECT_EQ(Name("::CE::Object"), Name("CE::Object::"));
    
    EXPECT_EQ(Name("::CE::Object").GetString(),   "CE::Object");
    EXPECT_EQ(Name("CE::Object::").GetString(),   "CE::Object");
    EXPECT_EQ(Name("::CE::Object").GetString(),   "CE::Object");
    EXPECT_EQ(Name("::CE::Object").GetString(),   "CE::Object");
    EXPECT_EQ(Name("::CE::Object::").GetString(), "CE::Object");
    EXPECT_FALSE(Name("::").IsValid());

	Name testPath = "/Game/Assets/Textures/Wood_Floor/Wood_Floor_Albedo";
	EXPECT_EQ(testPath.GetParentPath(), "/Game/Assets/Textures/Wood_Floor");

    TEST_END;
}

TEST(Containers, DateTime)
{
	TEST_BEGIN;

	DateTime utcTime = DateTime::UtcNow();
	DateTime localTime = DateTime::Now();

	{
		DateTime now = DateTime::Now();
		DateTime copy = now;
		copy.AddSeconds(86'401);

		//EXPECT_EQ(copy.Day(), now.Day() + 1);
		//EXPECT_EQ(copy.Second(), now.Second() + 1);

		u64 num = copy.ToNumber();
		EXPECT_EQ(DateTime::FromNumber(num).ToNumber(), num);

		EXPECT_EQ(DateTime::FromNumber(num), copy);
	}

	{
		DateTime now = DateTime::Now();

		String str = now.ToString();
		DateTime parsed = DateTime::Parse(str);

		EXPECT_EQ(now, parsed);
		EXPECT_EQ(parsed.ToString(), str);
	}

	TEST_END;
}

TEST(Containers, Variant)
{
	TEST_BEGIN;
	CERegisterModuleTypes();

	// String
	Variant value = "TestString";
	EXPECT_TRUE(value.IsOfType<String>());
	EXPECT_FALSE(value.IsOfType<Name>());
	EXPECT_FALSE(value.IsPointer());

	// Name
	value = TYPENAME(Object);
	EXPECT_TRUE(value.IsOfType<Name>());
	EXPECT_FALSE(value.IsOfType<String>());
	EXPECT_FALSE(value.IsPointer());

	value = Array<String>{ "Item1", "Item2" };
	EXPECT_TRUE(value.IsOfType<Array<u8>>());
	EXPECT_TRUE(value.IsOfType<Array<f32>>());
	EXPECT_TRUE(value.IsOfType<Array<String>>());
	EXPECT_EQ(value.GetValue<Array<String>>().GetElementTypeId(), TYPEID(String));
	EXPECT_EQ(value.GetValue<Array<f32>>().GetElementTypeId(), TYPEID(String));
	EXPECT_NE(value.GetValue<Array<String>>().GetElementTypeId(), TYPEID(f32));
	EXPECT_NE(value.GetValue<Array<f32>>().GetElementTypeId(), TYPEID(f32));
	EXPECT_EQ(value.GetValue<Array<String>>()[0], "Item1");
	EXPECT_EQ(value.GetValue<Array<String>>()[1], "Item2");

	// Object
	Object* testObject = CreateObject<Object>(nullptr, "TestObject");
	{
		Object& ref = *testObject;
		value = ref;

		EXPECT_TRUE(value.IsOfType<Object>());
		EXPECT_EQ(value.GetValue<Object*>(), testObject);

		auto& getValue = value.GetValue<Object&>();
		EXPECT_EQ(ref.GetUuid(), getValue.GetUuid());
	}
	testObject->BeginDestroy(); testObject = nullptr;

	// Struct
	{
		VariantStruct testStruct{};
		testStruct.stringArray = { "Item0", "Item1", "Item2" };
		testStruct.stringData = "Test String";

		value = testStruct;
		EXPECT_TRUE(value.IsStruct());

		testStruct.stringData = "New String";
		testStruct.stringArray = { "NewItem0", "NewItem1" };

		VariantStruct copyStruct = value.GetValue<VariantStruct>();
		EXPECT_EQ(copyStruct.stringData, "Test String");
		EXPECT_EQ(copyStruct.stringArray.GetSize(), 3);
		EXPECT_EQ(copyStruct.stringArray[0], "Item0");
		EXPECT_EQ(copyStruct.stringArray[1], "Item1");
		EXPECT_EQ(copyStruct.stringArray[2], "Item2");

		Variant variant2 = value;
		VariantStruct variant2Struct = variant2.GetValue<VariantStruct>();
		EXPECT_EQ(variant2Struct.stringData, "Test String");
		EXPECT_EQ(variant2Struct.stringArray.GetSize(), 3);
		EXPECT_EQ(variant2Struct.stringArray[0], "Item0");
		EXPECT_EQ(variant2Struct.stringArray[1], "Item1");
		EXPECT_EQ(variant2Struct.stringArray[2], "Item2");

		value = Variant();
		EXPECT_FALSE(value.HasValue());
	}

	// Struct RTTI
	{
		EXPECT_FALSE(value.HasValue());

		StructType* type = VariantStruct::StaticStruct();

		{
			VariantStruct testStruct{};
			testStruct.stringArray = { "Item 0", "Item 1", "Item 2" };
			testStruct.stringData = "Some String";

			value = type->CreateVariant(&testStruct);
		}

		EXPECT_TRUE(value.HasValue());
		EXPECT_TRUE(value.IsStruct());
		EXPECT_TRUE(value.IsOfType<VariantStruct>());
		EXPECT_EQ(value.GetValueTypeId(), type->GetTypeId());

		{
			VariantStruct testStruct{};

			type->LoadFromVariant(value, &testStruct);

			EXPECT_EQ(testStruct.stringArray.GetSize(), 3);
			EXPECT_EQ(testStruct.stringArray[0], "Item 0");
			EXPECT_EQ(testStruct.stringArray[1], "Item 1");
			EXPECT_EQ(testStruct.stringArray[2], "Item 2");
			EXPECT_EQ(testStruct.stringData, "Some String");
		}
	}

	CEDeregisterModuleTypes();
	TEST_END;
}

bool Vec4Equals(const Vec4& lhs, const Vec4& rhs)
{
	constexpr f32 epsilon = std::numeric_limits<f32>::epsilon();
	return Math::Abs(lhs.x - rhs.x) <= epsilon &&
		Math::Abs(lhs.y - rhs.y) <= epsilon &&
			Math::Abs(lhs.z - rhs.z) <= epsilon &&
				Math::Abs(lhs.w - rhs.w) <= epsilon;
}

TEST(Containers, Matrix)
{
    TEST_BEGIN;
    
	// Quaternion basics
	{
		Quat a = Quat(2, 3, 4, 1);
		Quat b = Quat(3, 4, 5, 2);

		Quat sum = a + b;
		EXPECT_EQ(sum, Quat(5, 7, 9, 3));

		Quat diff = b - a;
		EXPECT_EQ(diff, Quat(1, 1, 1, 1));

		Quat rotator = Quat::EulerDegrees(90, 0, 0);

		Vec4 vec = Vec4(0, 1, 0, 1);
		Vec4 out = rotator * vec;
		EXPECT_EQ(out, Vec4(0, 0, 1, 1));

		rotator = Quat::EulerDegrees(90, 0, 0) * rotator;
		vec = Vec4(0, 1, 0, 1);
		out = rotator * vec;
    	EXPECT_TRUE(Vec4Equals(out, Vec4(0, -1, 0, 1)));

		Quat lookRotation = Quat::LookRotation2(Vec3(1, 0, 0), Vec3(0, 1, 0));
		Vec3 degrees = lookRotation.ToEulerDegrees();

		degrees.GetNormalized();
	}

    // Multiplication
    {
        Vec3 pos = Vec3(10, 20, 30);
        
        auto translation = Matrix4x4({
            { 1, 0, 0, 10 },
            { 0, 1, 0, 0  },
            { 0, 0, 1, 0  },
            { 0, 0, 0, 1  },
        });
        
        Vec4 out = translation * Vec4(pos, 1.0f);
        EXPECT_EQ(out, Vec4(20, 20, 30, 1.0f));
    }
    
    // Transformation
    {
        auto translation = Matrix4x4({
            { 1, 0, 0, 10 },
            { 0, 1, 0, 0  },
            { 0, 0, 1, 0  },
            { 0, 0, 0, 1  },
        });
        
		Quat quat = Quat::EulerDegrees(0, 90, 0);
		auto rotation = quat.ToMatrix();

		auto scale = Matrix4x4({
			{ 2, 0, 0, 0 },
			{ 0, 2, 0, 0 },
			{ 0, 0, 2, 0 },
			{ 0, 0, 0, 1 },
		});

		Vec3 point = Vec3(1, 0, 0);

		Vec3 out = translation * rotation * scale * point;
		EXPECT_EQ(out, Vec3(10, 0, -2));

		Matrix4x4 transform = translation * rotation * scale;
		out = transform * point;
		EXPECT_EQ(out, Vec3(10, 0, -2));

		transform.Invert();
		out = transform * out;
		EXPECT_EQ(out, point);
    }
    
    TEST_END;
}

TEST(Containers, Sorting)
{
	TEST_BEGIN;

	// Custom sorting
	{
		struct Interval {
			int start, end;
		};

		Array<Interval> array = { { 6, 8 }, { 1, 9 }, { 2, 4 }, { 4, 7 } };
		array.Sort([](Interval i1, Interval i2)
			{
				return (i1.start < i2.start);
			});

		for (int i = 0; i < array.GetSize() - 1; i++)
		{
			auto cur = array[i];
			auto next = array[i + 1];
			EXPECT_LE(cur.start, next.start);
		}
	}

	// Filename sorting 1
	{
		Array<String> array = {
			"123",
			"92",
			"2",
			"Fbcd",
			"a421.png",
			"Zf12.jpg",
			"Z031.jpg",
			"@f12",
		};

		array.Sort(&String::NaturalCompare);
		
		int i = 0;
		EXPECT_EQ(array[i++], "2");
		EXPECT_EQ(array[i++], "92");
		EXPECT_EQ(array[i++], "123");
		EXPECT_EQ(array[i++], "@f12");
		EXPECT_EQ(array[i++], "a421.png");
		EXPECT_EQ(array[i++], "Fbcd");
		EXPECT_EQ(array[i++], "Z031.jpg");
		EXPECT_EQ(array[i++], "Zf12.jpg");
	}

	// Filename sorting 2
	{
		Array<String> array = {
			"Perlin_22",
			"Perlin_2",
			"Perlin_20",
			"Perlin_10",
			"Perlin_5",
			"Perlin_8",
			"Perlin_1",
		};

		array.Sort(&String::NaturalCompare);

		int i = 0;
		EXPECT_EQ(array[i++], "Perlin_1");
		EXPECT_EQ(array[i++], "Perlin_2");
		EXPECT_EQ(array[i++], "Perlin_5");
		EXPECT_EQ(array[i++], "Perlin_8");
		EXPECT_EQ(array[i++], "Perlin_10");
		EXPECT_EQ(array[i++], "Perlin_20");
		EXPECT_EQ(array[i++], "Perlin_22");
	}

	TEST_END;
}

TEST(Containers, Defer)
{
	TEST_BEGIN;

	String string = "original";
	EXPECT_EQ(string, "original");
	
	// Inner scope
	{
		defer(&) {
			string = "modified";
		};
		EXPECT_EQ(string, "original");

		string = "something";
		EXPECT_EQ(string, "something");
	}

	EXPECT_EQ(string, "modified");

	TEST_END;
}

TEST(Containers, ArrayView)
{
	TEST_BEGIN;

	// View into Array<int>
	{
		Array<int> array = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

		ArrayView<int> view = ArrayView<int>(array.Begin() + 2, array.End() - 2);
		EXPECT_EQ(view.GetSize(), 7);
		int idx = 2;
		EXPECT_EQ(view[0], 2);
		EXPECT_EQ(view[3], 5);
		EXPECT_EQ(view[6], 8);

		for (int value : view)
		{
			EXPECT_EQ(value, array[idx++]);
		}
	}

	// View into Array<String>
	{
		Array<String> array = { "str0", "str1", "str2", "str3", "str4" };

		ArrayView<String> view = ArrayView<String>(array.Begin() + 1, array.End() - 2);

		EXPECT_EQ(view.GetSize(), 3);
		EXPECT_EQ(view[0], "str1");
	}

	// View into initializer list
	{
		ArrayView<int> view = ArrayView<int>({ 0, 1, 2, 3, 4 });

		EXPECT_EQ(view.GetSize(), 5);
		EXPECT_EQ(view[0], 0);
		EXPECT_EQ(view[4], 4);
	}

	TEST_END;
}

TEST(Containers, FixedArray)
{
	TEST_BEGIN;

	FixedArray<String, 8> array = { "str0", "str1", "str2", "str3" };
	EXPECT_EQ(array.GetSize(), 4);
	EXPECT_EQ(array.GetCapacity(), 8);
	EXPECT_EQ(array[0], "str0");
	EXPECT_EQ(array[1], "str1");
	EXPECT_EQ(array[2], "str2");
	EXPECT_EQ(array[3], "str3");

	EXPECT_EQ(array.GetFirst(), "str0");
	EXPECT_EQ(array.GetLast(), "str3");

	array.Insert("str_Insert", 2);
	EXPECT_EQ(array.GetSize(), 5);
	EXPECT_EQ(array[0], "str0");
	EXPECT_EQ(array[1], "str1");
	EXPECT_EQ(array[2], "str_Insert");
	EXPECT_EQ(array[3], "str2");
	EXPECT_EQ(array[4], "str3");

	array.Remove("str_Insert");
	EXPECT_EQ(array[0], "str0");
	EXPECT_EQ(array[1], "str1");
	EXPECT_EQ(array[2], "str2");
	EXPECT_EQ(array[3], "str3");

	int i = 0;
	for (const String& item : array)
	{
		EXPECT_EQ(item, String("str") + i);
		i++;
	}

	TEST_END;
}

struct PagedElement
{
	String name = "";
	int index = 0;
};

TEST(Containers, PagedDynamicArray)
{
	TEST_BEGIN;

	using PagedArrayType = PagedDynamicArray<PagedElement, 4>;
	PagedArrayType array{};
	using ArrayHandle = PagedArrayType::Handle;
	struct HandleStruct
	{
		ArrayHandle handle;
	};
	Array<HandleStruct*> handles{};

	constexpr int totalCount = 32;

	// Insert & Remove

	for (int i = 0; i < totalCount; ++i)
	{
		HandleStruct* handleEntry = new HandleStruct();
		PagedElement element = PagedElement{ .name = String::Format("Element {}", i), .index = i };
		handleEntry->handle = array.Insert(element);
		handles.Add(handleEntry);
	}
	
	EXPECT_EQ(array.GetPageCount(), totalCount / 4);
	EXPECT_EQ(array.GetCount(), totalCount);

	HashSet<int> foundValues{};

	delete handles[12]; handles[12] = nullptr;
	delete handles[6]; handles[6] = nullptr;
	delete handles[2]; handles[2] = nullptr;
	delete handles[0]; handles[0] = nullptr;

	for (auto it = array.begin(); it != array.end(); ++it)
	{
		foundValues.Add(it->index);
	}

	for (int i = 0; i < totalCount; ++i)
	{
		if (i == 0 || i == 2 || i == 6 || i == 12)
		{
			EXPECT_FALSE(foundValues.Exists(i));
		}
		else
		{
			EXPECT_TRUE(foundValues.Exists(i));
		}
	}

	// Page Iterator

	auto parallelRanges = array.GetParallelRanges();

	for (int i = 0; i < parallelRanges.GetSize(); ++i)
	{
		const auto& parallelRange = parallelRanges[i];

		for (auto it = parallelRange.begin; it != parallelRange.end; ++it)
		{
			EXPECT_EQ(it.GetPageIndex(), i);

			EXPECT_NE(it->index, 0);
			EXPECT_NE(it->index, 2);
			EXPECT_NE(it->index, 6);
			EXPECT_NE(it->index, 12);

			EXPECT_TRUE(foundValues.Exists(it->index));
		}
	}

	// Reuse empty slots

	{
		handles[0] = new HandleStruct();
		PagedElement element = PagedElement{ .name = "Element 0", .index = 0 };
		handles[0]->handle = array.Insert(element);

		handles[2] = new HandleStruct();
		PagedElement element2 = PagedElement{ .name = "Element 2", .index = 2 };
		handles[2]->handle = array.Insert(element2);
	}

	foundValues.Clear();
	for (auto it = array.begin(); it != array.end(); ++it)
	{
		foundValues.Add(it->index);
	}

	for (int i = 0; i < totalCount; ++i)
	{
		if (i == 6 || i == 12)
		{
			EXPECT_FALSE(foundValues.Exists(i));
		}
		else
		{
			EXPECT_TRUE(foundValues.Exists(i));
		}
	}

	// - Cleanup -

	for (int i = 0; i < handles.GetSize(); i++)
	{
		if (handles[i] != nullptr)
		{
			delete handles[i];
		}
	}
	handles.Clear();

	EXPECT_EQ(array.GetCount(), 0);

	TEST_END;
}

#pragma endregion


/**********************************************
*   Reflection
*/

#pragma region Reflection

TEST(Reflection, TypeId)
{
    TEST_BEGIN;

    // 1. Templates
    
    static_assert(TIsTemplate<Array<String>>::Value);
    static_assert(!TIsTemplate<String>::Value);
    
    static_assert(TIsSameType<TemplateType<Array<String>>::DefaultArg, u8>::Value);
    
    EXPECT_NE(TYPE(Array<String>), nullptr);
    EXPECT_EQ(GetStaticType<Array<String>>(), GetStaticType<Array<u8>>());
    EXPECT_EQ(GetStaticType<Array<String>>(), GetStaticType<Array<Object*>>());

    // 2. Pointers & References

    EXPECT_EQ(TYPEID(Object*), TYPEID(Object));
    EXPECT_EQ(TYPEID(const Object*), TYPEID(Object));
    EXPECT_EQ(TYPEID(const Object*), TYPEID(const Object));
    EXPECT_EQ(TYPEID(const Object* const), TYPEID(const Object));

    EXPECT_EQ(TYPEID(f32), TYPEID(f32*));

    EXPECT_EQ(TYPEID(Object&), TYPEID(Object));
    EXPECT_EQ(TYPEID(const Object&), TYPEID(Object));
    EXPECT_EQ(TYPEID(s32&), TYPEID(s32));

    // 3. Arrays

    EXPECT_EQ(TYPEID(Array<String>), TYPEID(Array<u8>));

    TEST_END;
}


enum class CustomFlagsEnum
{
	None = 0,
	Transient = BIT(0),
	Active = BIT(1),
	Focused = BIT(2),
	Hovered = BIT(3),
	Pressed = BIT(4),
};
ENUM_CLASS_FLAGS(CustomFlagsEnum);

CE_RTTI_ENUM(, , CustomFlagsEnum,
	CE_ATTRIBS(Flags),
	CE_CONST(None),
	CE_CONST(Transient),
	CE_CONST(Active),
	CE_CONST(Focused),
	CE_CONST(Hovered),
	CE_CONST(Pressed),
);
CE_RTTI_ENUM_IMPL(, , CustomFlagsEnum);

TEST(Reflection, RTTI_Registry_Testing)
{
    // 1. Test Loading & Unloading

    auto registeredCount = TypeInfo::GetRegisteredCount();
    EXPECT_EQ(registeredCount, 0); // == 0
    EXPECT_EQ(ClassType::GetTotalRegisteredClasses(), 0); // == 0
    EXPECT_EQ(ClassType::FindClass(TYPEID(Object)), nullptr);

    ModuleManager::Get().LoadModule("Core");
    registeredCount = TypeInfo::GetRegisteredCount();
    EXPECT_GT(registeredCount, 0); // > 0
    EXPECT_NE(ClassType::FindClass(TYPEID(Object)), nullptr);

    ModuleManager::Get().UnloadModule("Core");
    EXPECT_EQ(TypeInfo::GetRegisteredCount(), 0); // == 0

    ModuleManager::Get().LoadModule("Core");
    EXPECT_EQ(TypeInfo::GetRegisteredCount(), registeredCount); // should be equal

    // 2. POD Testing

    EXPECT_EQ(TYPENAME(s32), "/Code/Core.s32");
    EXPECT_EQ(TYPENAME(Array<String>), "/Code/Core.CE::Array");
    
    // 3. Containers RTTI
    
    EXPECT_EQ(TYPEID(Array<String>), TYPEID(Array<Object*>));

	// 4. Enum
	auto enumType = TYPE(EventResult);
	EXPECT_NE(enumType, nullptr);
	EXPECT_EQ(enumType->GetName(), "CE::EventResult");
	EXPECT_EQ(enumType->GetTypeName(), "/Code/Core.CE::EventResult");

	CE_REGISTER_TYPES(CustomFlagsEnum);
    {
		CustomFlagsEnum customFlags = CustomFlagsEnum::Active | CustomFlagsEnum::Pressed | CustomFlagsEnum::Transient;
		String string = String::Format("Value: {}", customFlags);

		EXPECT_EQ(string, "Value: Transient|Active|Pressed");

		EventResult eventResult = EventResult::HandleAndStopPropagation;
		string = String::Format("Value: {}", eventResult);
		EXPECT_EQ(string, "Value: HandleAndStopPropagation");
    }
	CE_DEREGISTER_TYPES(CustomFlagsEnum);

    // 5. Test Object class

    auto objectClass = ClassType::FindClass(TYPENAME(Object));
    EXPECT_NE(objectClass, nullptr);
    EXPECT_EQ(objectClass, GetStaticType<Object>());

    if (objectClass != nullptr)
    {
        EXPECT_EQ(objectClass->GetTypeName(), TYPENAME(Object)); // equal
        EXPECT_GE(objectClass->GetFieldCount(), 2); // >= 2

        auto derivedClasses = objectClass->GetDerivedClasses();
        Array<TypeId> loopedClasses{};

        for (auto derivedClass : derivedClasses) // Make sure no duplicates exist
        {
            EXPECT_NE(derivedClass, nullptr);
            EXPECT_FALSE(loopedClasses.Exists(derivedClass->GetTypeId()));
            loopedClasses.Add(derivedClass->GetTypeId());
        }
    }

    DeregisterTypes<Object>();
    // You cannot manually deregister types that were automatically registered
    EXPECT_NE(ClassType::FindClass(TYPEID(Object)), nullptr);

    ModuleManager::Get().UnloadModule("Core");
    EXPECT_EQ(TypeInfo::GetRegisteredCount(), 0);
}

class Core_Reflection_Attribute_Test_Class : public CE::Object
{
    CE_CLASS(Core_Reflection_Attribute_Test_Class, CE::Object)
public:

};

CE_RTTI_CLASS(, , Core_Reflection_Attribute_Test_Class,
    CE_SUPER(CE::Object),
    CE_NOT_ABSTRACT,
    CE_ATTRIBS(DisplayName = "My Class Display Name", meta = (Id = 4124, Tooltip = "A tooltip description"), Redirects = (SomeOldName, "Old Name")),
    CE_FIELD_LIST(),
    CE_FUNCTION_LIST()
)

CE_RTTI_CLASS_IMPL(, , Core_Reflection_Attribute_Test_Class)

TEST(Reflection, Attribute_Parsing)
{
    TEST_BEGIN;

    CE_REGISTER_TYPES(Core_Reflection_Attribute_Test_Class);
    EXPECT_NE(GetTypeInfo(TYPEID(Core_Reflection_Attribute_Test_Class)), nullptr);

    auto classType = GetStaticClass<Core_Reflection_Attribute_Test_Class>();
    auto attrib = classType->GetAttributes();
    EXPECT_TRUE(attrib.HasKey("DisplayName"));
    EXPECT_TRUE(classType->GetAttribute("DisplayName").IsString());
    EXPECT_EQ(classType->GetAttribute("DisplayName").GetStringValue(), "My Class Display Name");

    EXPECT_TRUE(classType->HasAttribute("meta"));
    EXPECT_TRUE(classType->GetAttribute("meta").IsMap());
    EXPECT_EQ(classType->GetAttribute("meta")["Id"].GetStringValue(), "4124");
    EXPECT_EQ(classType->GetAttribute("meta")["Tooltip"].GetStringValue(), "A tooltip description");

    EXPECT_TRUE(classType->HasAttribute("Redirects"));
    EXPECT_TRUE(classType->GetAttribute("Redirects").IsMap());
    EXPECT_TRUE(classType->GetAttribute("Redirects").HasKey("SomeOldName"));
    EXPECT_EQ(classType->GetAttribute("Redirects")["SomeOldName"].GetStringValue(), "");
    EXPECT_TRUE(classType->GetAttribute("Redirects").HasKey("Old Name"));
    EXPECT_EQ(classType->GetAttribute("Redirects")["Old Name"].GetStringValue(), "");

    CE_DEREGISTER_TYPES(Core_Reflection_Attribute_Test_Class);
    EXPECT_EQ(GetTypeInfo(TYPEID(Core_Reflection_Attribute_Test_Class)), nullptr);

    TEST_END;
}

TEST(Reflection, SubClassType)
{
	TEST_BEGIN;
	CERegisterModuleTypes();
	using namespace ObjectTests;

	// 1. Basic test
	{
		SubClassType<BaseClass> subclass{};
		subclass = Object::Type();
		EXPECT_EQ(subclass, nullptr);

		subclass = DerivedClassA::Type();
		EXPECT_EQ(subclass, DerivedClassA::Type());

		subclass = Object::Type();
		EXPECT_EQ(subclass, DerivedClassA::Type());

		subclass = nullptr;
		EXPECT_EQ(subclass, nullptr);

		// Get reference with a different template class
		// Original subclass requirement should be maintained
		{
			SubClassType<Object>& ref = subclass;
			ref = Object::Type();
			EXPECT_EQ(ref, nullptr); EXPECT_EQ(subclass, nullptr);

			ref = BaseClass::Type();
			EXPECT_EQ(ref, BaseClass::Type()); EXPECT_EQ(subclass, BaseClass::Type());

			ref = DerivedClassA::Type();
			EXPECT_EQ(ref, DerivedClassA::Type()); EXPECT_EQ(subclass, DerivedClassA::Type());

			ref = nullptr;
			EXPECT_EQ(ref, nullptr); EXPECT_EQ(subclass, nullptr);
		}

		// Get reference with DerivedClassA as template class
		{
			SubClassType<DerivedClassA>& ref = subclass;
			ref = Object::Type();
			EXPECT_EQ(ref, nullptr); EXPECT_EQ(subclass, nullptr);

			ref = BaseClass::Type();
			EXPECT_EQ(ref, BaseClass::Type()); EXPECT_EQ(subclass, BaseClass::Type());

			ref = DerivedClassA::Type();
			EXPECT_EQ(ref, DerivedClassA::Type()); EXPECT_EQ(subclass, DerivedClassA::Type());

			ref = nullptr;
			EXPECT_EQ(ref, nullptr); EXPECT_EQ(subclass, nullptr);
		}
	}

	// 2. Field & RTTI
	{
		ClassType* type = DerivedClassA::Type();

		EXPECT_EQ(TYPEID(SubClassType<Object>), TYPEID(SubClassType<int>));
		EXPECT_EQ(TYPEID(SubClassType<Object>), TYPEID(SubClassType<void>));
		EXPECT_EQ(TYPEID(SubClassType<Object>), TYPEID(SubClassType<String>));
		EXPECT_EQ(TYPEID(SubClassType<Object>), TYPEID(SubClassType<DerivedClassA>));

		EXPECT_EQ(type->FindField("assignedClass")->GetDeclarationType(), TYPE(SubClassType<Object>));
		EXPECT_EQ(type->FindField("derivedClassType")->GetDeclarationType(), TYPE(SubClassType<Object>));
		EXPECT_EQ(type->FindField("anyClass")->GetDeclarationType(), TYPE(SubClassType<Object>));

		EXPECT_EQ(type->FindField("assignedClass")->GetUnderlyingTypeId(), TYPEID(BaseClass));
		EXPECT_EQ(type->FindField("derivedClassType")->GetUnderlyingTypeId(), TYPEID(DerivedClassA));
		EXPECT_EQ(type->FindField("anyClass")->GetUnderlyingTypeId(), TYPEID(Object));

		EXPECT_EQ(type->FindField("assignedClass")->GetUnderlyingType(), TYPE(BaseClass));
		EXPECT_EQ(type->FindField("derivedClassType")->GetUnderlyingType(), TYPE(DerivedClassA));
		EXPECT_EQ(type->FindField("anyClass")->GetUnderlyingType(), TYPE(Object));
	}

	CEDeregisterModuleTypes();
	TEST_END;
}

struct ReflectionFieldElement
{
	CE_STRUCT(ReflectionFieldElement)
public:

	static int releaseCount;

	virtual ~ReflectionFieldElement()
	{
		releaseCount++;
	}

	String myText = "default";
};

int ReflectionFieldElement::releaseCount = 1;

CE_RTTI_STRUCT(, , ReflectionFieldElement,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(myText)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(, , ReflectionFieldElement)

struct ReflectionFieldTest
{
	CE_STRUCT(ReflectionFieldTest)
public:

	Array<ReflectionFieldElement> array{};

	String testString = "default value";
};

CE_RTTI_STRUCT(,,ReflectionFieldTest,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(array)
		CE_FIELD(testString)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(,,ReflectionFieldTest)

class ReflectionFieldClass : public Object
{
	CE_CLASS(ReflectionFieldClass, Object)
public:

	Array<ReflectionFieldTest> structArray;

	Array<Ref<Object>> objectArray;

	Ref<Object> testObject;

};

CE_RTTI_CLASS(,,ReflectionFieldClass,
	CE_SUPER(CE::Object),
	CE_NOT_ABSTRACT,
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(structArray)
		CE_FIELD(objectArray)
		CE_FIELD(testObject)
	),
	CE_FUNCTION_LIST(
	)
)
CE_RTTI_CLASS_IMPL(,,ReflectionFieldClass)

TEST(Reflection, Fields)
{
	TEST_BEGIN;
	CE_REGISTER_TYPES(ReflectionFieldElement, ReflectionFieldTest, ReflectionFieldClass);
	CERegisterModuleTypes();

	// 1. Array field resize
	{
		ReflectionFieldTest data{};
		data.testString = "modified";
		StructType* type = data.GetStruct();
		auto arrayField = type->FindField("array");
		auto stringField = type->FindField("testString");

		arrayField->ResizeArray(&data, 3);
		EXPECT_EQ(data.array[0].myText, "default");

		EXPECT_EQ(data.testString, "modified");
		stringField->InitializeDefaults(&data);
		EXPECT_EQ(data.testString, "");
		type->InitializeDefaults(&data);
		EXPECT_EQ(data.testString, "default value");
	}

	// 2. Array field insertion & deletion
	{
		ReflectionFieldTest data{};
		StructType* type = data.GetStruct();
		auto arrayField = type->FindField("array");
		auto stringField = type->FindField("testString");

		ReflectionFieldElement::releaseCount = 0;
		EXPECT_EQ(ReflectionFieldElement::releaseCount, 0);

		arrayField->ResizeArray(&data, 1);
		EXPECT_EQ(data.array[0].myText, "default");
		data.array[0].myText = "new";
		EXPECT_EQ(data.array[0].myText, "new");

		arrayField->InsertArrayElement(&data, 1);
		EXPECT_EQ(data.array[0].myText, "new");
		EXPECT_EQ(data.array[1].myText, "default");
		data.array[1].myText = "new2";
		EXPECT_EQ(data.array[1].myText, "new2");

		arrayField->ResizeArray(&data, 3); // Adds 1 new element
		EXPECT_EQ(data.array[0].myText, "new");
		EXPECT_EQ(data.array[1].myText, "new2");
		EXPECT_EQ(data.array[2].myText, "default");

		arrayField->DeleteArrayElement(&data, 1); // Delete element at index 1
		EXPECT_EQ(data.array.GetSize(), 2);
		EXPECT_EQ(data.array[0].myText, "new");
		EXPECT_EQ(data.array[1].myText, "default");
		data.array[1].myText = "new2";

		arrayField->InsertArrayElement(&data, 1); // Insert new element at index 1
		EXPECT_EQ(data.array[0].myText, "new");
		EXPECT_EQ(data.array[1].myText, "default");
		EXPECT_EQ(data.array[2].myText, "new2");

		EXPECT_EQ(ReflectionFieldElement::releaseCount, 1);
		ReflectionFieldElement::releaseCount = 0;
	}

	// 3. Simple field relative path
	{
		ReflectionFieldTest data{};
		StructType* type = data.GetStruct();

		data.array.Resize(3);
		data.array[0].myText = "Data 0";
		data.array[1].myText = "Data 1";
		data.array[2].myText = "Data 2";
		data.testString = "Test";

		StructType* fieldOwner = nullptr;
		Ptr<FieldType> outField = nullptr;
		Ref<Object> outObject = nullptr;
		void* outInstance = nullptr;

		bool success = type->FindFieldInstanceRelative("array[2].myText", nullptr, &data, 
			fieldOwner, outField, outObject, outInstance);

		EXPECT_TRUE(success);

		String outValue = outField->GetFieldValue<String>(outInstance);
		EXPECT_EQ(outValue, "Data 2");
	}

	// 4. Advanced field relative path
	{
		Ref<ReflectionFieldClass> mainObject = CreateObject<ReflectionFieldClass>(nullptr, "TestObjectClass");
		ClassType* clazz = mainObject->GetClass();

		mainObject->structArray.Resize(2);
		mainObject->structArray[0].testString = "Item 0";
		{
			mainObject->structArray[0].array.Resize(2);
			mainObject->structArray[0].array[0].myText = "Item 0 Text 0";
			mainObject->structArray[0].array[1].myText = "Item 0 Text 1";
		}

		mainObject->structArray[1].testString = "Item 1";
		{
			mainObject->structArray[1].array.Resize(2);
			mainObject->structArray[1].array[0].myText = "Item 1 Text 0";
			mainObject->structArray[1].array[1].myText = "Item 1 Text 1";
		}

		mainObject->objectArray.Resize(2);
		mainObject->objectArray[0] = CreateObject<Object>(mainObject.Get(), "SubObject_1");
		mainObject->objectArray[1] = nullptr;

		mainObject->testObject = CreateObject<Object>(mainObject.Get(), "TestObject");

		StructType* fieldOwner = nullptr;
		Ptr<FieldType> outField = nullptr;
		Ref<Object> outObject = nullptr;
		void* outInstance = nullptr;

		bool success = clazz->FindFieldInstanceRelative("structArray[1].array[0].myText", mainObject.Get(), mainObject.Get(),
			fieldOwner, outField, outObject, outInstance);

		EXPECT_TRUE(success);
		EXPECT_EQ(outField->GetFieldValue<String>(outInstance), "Item 1 Text 0");

		success = clazz->FindFieldInstanceRelative("structArray[1].array[1].myText", mainObject.Get(), mainObject.Get(),
			fieldOwner, outField, outObject, outInstance);

		EXPECT_TRUE(success);
		EXPECT_EQ(outField->GetFieldValue<String>(outInstance), "Item 1 Text 1");

		success = clazz->FindFieldInstanceRelative("objectArray[0].name", mainObject.Get(), mainObject.Get(),
			fieldOwner, outField, outObject, outInstance);

		EXPECT_TRUE(success);
		EXPECT_EQ(outField->GetFieldValue<Name>(outInstance), mainObject->objectArray[0]->GetName());

		success = clazz->FindFieldInstanceRelative("objectArray[20].name", mainObject.Get(), mainObject.Get(),
			fieldOwner, outField, outObject, outInstance);

		EXPECT_FALSE(success);

		success = clazz->FindFieldInstanceRelative("structArray[1].array[3].myText", mainObject.Get(), mainObject.Get(),
			fieldOwner, outField, outObject, outInstance);

		EXPECT_FALSE(success);

		success = clazz->FindFieldInstanceRelative("objectArray[0]", mainObject.Get(), mainObject.Get(),
			fieldOwner, outField, outObject, outInstance);

		EXPECT_TRUE(success);
		EXPECT_TRUE(outField->GetFieldValue<Ref<Object>>(outInstance).IsValid());

		success = clazz->FindFieldInstanceRelative("objectArray[1]", mainObject.Get(), mainObject.Get(),
			fieldOwner, outField, outObject, outInstance);

		EXPECT_TRUE(success);
		EXPECT_TRUE(outField->GetFieldValue<Ref<Object>>(outInstance).IsNull());
	}



	CEDeregisterModuleTypes();
	CE_DEREGISTER_TYPES(ReflectionFieldElement, ReflectionFieldTest, ReflectionFieldClass);
	TEST_END;
}

#pragma endregion


/**********************************************
*   Config
*/

#pragma region Config

TEST(Config, BasicParsing)
{
    TEST_BEGIN;
    
    String fileName = "CoreTemporaryConfig.ini";
    auto configPath = PlatformDirectories::GetLaunchDir() / "Config" / fileName;
    if (configPath.Exists())
        IO::Path::Remove(configPath);
    
    IO::FileStream file(configPath, IO::OpenMode::ModeWrite);
    if (!file.IsOpen())
    {
        FAIL();
        return;
    }
    
    std::stringstream stream{};
    stream << "[CE::GeneralSettings]\n";
    stream << "ExampleBool=false\n";
    stream << "Bindings=(Name=\"Q\",Command=\"Foo\")\n";
    stream << "+Bindings=(Name=\"E\",Command=\"Too\")\n";
    stream << "!Bindings=Clear\n";
    stream << "+Bindings=(Name=\"A\",Command=\"Foo\")\n";
    stream << "+Bindings=(Name=\"B\",Command=\"Too\")\n";
    stream << "-Bindings=(Name=\"A\",Command=\"Foo\")\n";
    stream << "ExampleString=This is an example of long string\n";
    
    std::string str = stream.str();
    file.Write(str.size(), str.data());
    file.Close();
    
    ConfigFile configFile{};
    configFile.Read(configPath);
    
    EXPECT_EQ(configFile.GetSize(), 1);
    EXPECT_EQ(configFile["CE::GeneralSettings"].GetSize(), 3);
    EXPECT_EQ(configFile["CE::GeneralSettings"]["ExampleBool"].GetString(), "false");
    auto& bindingsArray = configFile["CE::GeneralSettings"]["Bindings"].GetArray();
    EXPECT_EQ(bindingsArray.GetSize(), 1);
    EXPECT_EQ(bindingsArray[0], "(Name=\"B\",Command=\"Too\")");
    EXPECT_TRUE(configFile["CE::GeneralSettings"].KeyExists("ExampleString"));
    EXPECT_EQ(configFile["CE::GeneralSettings"]["ExampleString"].GetString(), "This is an example of long string");
    
    IO::Path::Remove(configPath);
    
    TEST_END;
}

TEST(Config, HierarchicalParsing)
{
    TEST_BEGIN;

    auto engineConfig = gConfigCache->GetConfigFile(CFG_Test());

    EXPECT_TRUE(engineConfig->SectionExists("Test::0"));
    auto test0Section = engineConfig->Get("Test::0");
    
    EXPECT_EQ(test0Section.GetSize(), 4);
    EXPECT_TRUE(test0Section.KeyExists("boolValue"));
    EXPECT_TRUE(test0Section.KeyExists("floatValue"));
    EXPECT_TRUE(test0Section.KeyExists("stringValue"));
    EXPECT_TRUE(test0Section.KeyExists("arrayValue"));

    EXPECT_EQ(test0Section["boolValue"].GetString(), "true");
    EXPECT_EQ(test0Section["stringValue"].GetString(), "An even longer string value. This can be a description text.");
    EXPECT_EQ(test0Section["floatValue"].GetString(), "12.24");
    EXPECT_EQ(test0Section["arrayValue"].GetArray().GetSize(), 2);
    EXPECT_EQ(test0Section["arrayValue"].GetArray()[0], "NewItem0");
    EXPECT_EQ(test0Section["arrayValue"].GetArray()[1], "NewItem1");
    
    TEST_END;
}

#pragma endregion


/**********************************************
*   Object
*/

#pragma region Object

TEST(Object, TransientLifecycle)
{
	using namespace LifecycleTests;

	// 1. Transient

	TEST_BEGIN;
	CE_REGISTER_TYPES(LifecycleTests::LifecycleClass, LifecycleSubObject);

	WeakRef<Object> weakRef = nullptr;
	lifecycleClassDestroyed = false;

	Ref<Bundle> transient = GetGlobalTransient();

	{
		Ref<LifecycleClass> object = CreateObject<LifecycleClass>(transient.Get(), "TestObject");
		weakRef = object;
	}

	if (Ref<Object> obj = weakRef.Lock())
	{

	}
	else
	{
		FAIL();
	}

	CE_DEREGISTER_TYPES(LifecycleTests::LifecycleClass, LifecycleSubObject);
	TEST_END

	EXPECT_TRUE(lifecycleClassDestroyed);
	lifecycleClassDestroyed = false;
	EXPECT_TRUE(weakRef.IsNull());

	// 2. BeginDestroy()

	TEST_BEGIN;
	CE_REGISTER_TYPES(LifecycleTests::LifecycleClass, LifecycleSubObject);

	{
		Ref<Bundle> transient = GetGlobalTransient();

		Ref<LifecycleClass> object = CreateObject<LifecycleClass>(transient.Get(), "TestObject");
		weakRef = object;

		object->BeginDestroy();
	}

	EXPECT_TRUE(lifecycleClassDestroyed);
	lifecycleClassDestroyed = false;
	EXPECT_TRUE(weakRef.IsNull());

	CE_DEREGISTER_TYPES(LifecycleTests::LifecycleClass, LifecycleSubObject);
	TEST_END
}

TEST(Object, Lifecycle)
{
	using namespace LifecycleTests;

	TEST_BEGIN;
	CE_REGISTER_TYPES(LifecycleTests::LifecycleClass, LifecycleSubObject);

	EXPECT_EQ(TYPEID(Ref<LifecycleClass>), TYPEID(LifecycleClass));
	EXPECT_EQ(TYPEID(WeakRef<LifecycleClass>), TYPEID(LifecycleClass));
	EXPECT_EQ(TYPEID(WeakRef<LifecycleClass>), TYPEID(Ref<LifecycleClass>));

    Object* rawRef = nullptr;
	Object* rawRef2 = nullptr;
    
    // 1. Simple ref counting
    {
		//Ref<Bundle> transient = GetGlobalTransient();

        Ref<Object> object = CreateObject<Bundle>(nullptr, "TestBundle");
        rawRef = object.Get();
        
        Ref<Bundle> bundle = Object::CastTo<Bundle>(object.Get());
        
        Ref<Object> object2 = CreateObject<Object>(nullptr, "TestObj");
		rawRef2 = object2.Get();
        
        Ref<Object> nullRef = nullptr;
        bool exception = false;
        
        try {
            //nullRef->BeginDestroy();
        } catch (const NullPointerException& exc) {
            exception = true;
        }
        
        //EXPECT_TRUE(exception);
        exception = false;
        
        WeakRef<Object> weakRef1 = object;
        WeakRef<Bundle> weakRef2 = bundle;
        WeakRef<Object> weakRef3 = object2;
        EXPECT_EQ(weakRef1->GetName(), "TestBundle");
        EXPECT_EQ(weakRef3->GetName(), "TestObj");
        
        if (Ref<Object> obj = weakRef2.Lock())
        {
            
        }
        else
        {
            FAIL();
        }
        
        object = nullptr;
        bundle = nullptr;
        object2 = nullptr;
        
        try {
            //weakRef1->AddToRoot();
        } catch (const NullPointerException& exc) {
            exception = true;
        }
        
        //EXPECT_TRUE(exception);
        exception = false;
        
        if (Ref<Object> obj = weakRef2.Lock())
        {
            FAIL();
        }

		weakRef1 = nullptr;
		weakRef2 = nullptr;
		weakRef3 = nullptr;
    }

	// 2. Complex test
	{
		lifecycleSubObjectDestroyed = lifecycleClassDestroyed = false;
		lifecycleSubObjectDestroyCount = 0;

		Ref<LifecycleClass> object = CreateObject<LifecycleClass>(nullptr, "LifecycleClass");
		rawRef = object.Get();

		Ref<LifecycleSubObject> subObject = object->subobject;
		WeakRef<LifecycleClass> weak1 = object;
		WeakRef<LifecycleSubObject> weak2 = subObject;
		Ref<LifecycleSubObject> strongRefToChild = nullptr;
		WeakRef<LifecycleSubObject> weakRefToChild = nullptr;

		for (int i = 0; i < 4; ++i)
		{
			Ref<LifecycleSubObject> child = CreateObject<LifecycleSubObject>(object.Get(), String::Format("Array_{}", i));
			rawRef = child.Get();

			if (i == 0)
			{
				strongRefToChild = child;
			}
			else if (i == 1)
			{
				weakRefToChild = child;
			}

			if (i % 2 == 0) // 2 different ways to insert array elements
			{
				object->childArray.Add(child);
			}
			else
			{
				FieldType* arrayField = object->GetClass()->FindField("childArray");
				EXPECT_TRUE(arrayField->IsWeakRefCounted());
				EXPECT_TRUE(arrayField->IsRefCounted());
				arrayField->InsertArrayElement(object.Get());
				arrayField->SetArrayElementValueAt<WeakRef<Object>>(i, object.Get(), child);
			}
		}
		
		// Destroy object
		object = nullptr;

		if (Ref<LifecycleSubObject> lock = weakRefToChild.Lock())
		{
			FAIL();
		}

		EXPECT_TRUE(lifecycleClassDestroyed);
		EXPECT_TRUE(subObject->IsPendingDestruction());
		EXPECT_TRUE(strongRefToChild->IsPendingDestruction());
	}
	EXPECT_TRUE(lifecycleSubObjectDestroyed);
	EXPECT_TRUE(lifecycleClassDestroyed);
	EXPECT_EQ(lifecycleSubObjectDestroyCount, 5);
    
    // 3. Multithreading
    {
		lifecycleSubObjectDestroyed = lifecycleClassDestroyed = false;
		lifecycleSubObjectDestroyCount = 0;

		Ref<LifecycleClass> rootObject = CreateObject<LifecycleClass>(nullptr, "Lifecycle");
		WeakRef<LifecycleClass> weakRef = rootObject;
		constexpr int NumThreads = 16;
		StaticArray<Thread, NumThreads> threads;
		std::atomic<int> counter = 0;

		for (int i = 0; i < NumThreads; ++i)
		{
			int idx = i;
			threads[i] = [weakRef, &counter, idx, &rootObject]
				{
					if (idx >= NumThreads / 2)
					{
						Thread::SleepFor(Random::Range(50, 100));
					}

					// ReSharper disable once CppTooWideScope
					Ref<LifecycleClass> strongRef = weakRef.Lock();
					if (idx == 2)
					{
						rootObject = nullptr;
					}
					if (strongRef)
					{
						++counter;
					}
				};
		}

		for (int i = 0; i < NumThreads; ++i)
		{
			threads[i].Join();
		}

		int count = counter;
		EXPECT_LE(count, NumThreads);
		EXPECT_TRUE(weakRef.IsNull());
    }

	CE_DEREGISTER_TYPES(LifecycleClass, LifecycleSubObject);
    TEST_END;
}


namespace ObjectTest
{
	struct CDIStruct
	{
		CE_STRUCT(CDIStruct)
	public:
		String name = "";
		u32 value = 0;
	};

	class CDISubClass : public Object
	{
		CE_CLASS(CDISubClass, Object)
	public:

		void OnSubobjectAttached(Object* subobject) override
		{
			Super::OnSubobjectAttached(subobject);
		}

		String subString = "sub-string";

		CDISubClass* subClass = nullptr;
	};

    class CDITest : public Object
    {
        CE_CLASS(CDITest, Object)
    public:

		CDITest()
		{
			subClass = CreateDefaultSubobject<CDISubClass>("SubClassObject");
			subClass->subString = "modified";
		}

		void OnSubobjectAttached(Object* subobject) override
		{
			Super::OnSubobjectAttached(subobject);
		}
        
		f32 floatValue = 1.21f;
		String stringValue = "none";
		Array<String> arrayValue{};
		bool boolValue = false;

		CDISubClass* subClass = nullptr;

		Array<CDIStruct> dictionary{};
    };
}

CE_RTTI_STRUCT(, ObjectTest, CDIStruct,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(name)
		CE_FIELD(value)
	),
    CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(, ObjectTest, CDIStruct)

CE_RTTI_CLASS(, ObjectTest, CDISubClass,
	CE_SUPER(CE::Object),
	CE_NOT_ABSTRACT,
	CE_ATTRIBS(Config = Test),
	CE_FIELD_LIST(
		CE_FIELD(subString, Config)
		CE_FIELD(subClass)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_CLASS_IMPL(, ObjectTest, CDISubClass)

CE_RTTI_CLASS(, ObjectTest, CDITest,
    CE_SUPER(CE::Object),
    CE_NOT_ABSTRACT,
    CE_ATTRIBS(Config = Test),
    CE_FIELD_LIST(
		CE_FIELD(floatValue, Config)
		CE_FIELD(stringValue, Config)
		CE_FIELD(arrayValue, Config)
		CE_FIELD(boolValue, Config)
		CE_FIELD(subClass)
		CE_FIELD(dictionary, Config)
	),
    CE_FUNCTION_LIST()
)
CE_RTTI_CLASS_IMPL(, ObjectTest, CDITest)

TEST(Object, CDI)
{
    TEST_BEGIN;
	CE_REGISTER_TYPES(CDITest, CDIStruct, CDISubClass);

	{
		CDISubClass* testSubClass = CreateObject<CDISubClass>(nullptr, "CDISubClassTest");
		EXPECT_EQ(testSubClass->subString, "String from ini");

		testSubClass->BeginDestroy();
	}
	
	CDITest* cdi = GetMutableDefaults<CDITest>();
	EXPECT_NE(cdi, nullptr);
	EXPECT_EQ(cdi->floatValue, 42.21f);
	EXPECT_EQ(cdi->stringValue, "A sample string value");
	EXPECT_EQ(cdi->boolValue, true);
	
	EXPECT_EQ(cdi->arrayValue.GetSize(), 3);
	EXPECT_EQ(cdi->arrayValue[0], "Entry0");
	EXPECT_EQ(cdi->arrayValue[1], "Entry1");
	EXPECT_EQ(cdi->arrayValue[2], "Entry2");

	EXPECT_EQ(cdi->dictionary.GetSize(), 3);
	EXPECT_EQ(cdi->dictionary[0].name, "id0");
	EXPECT_EQ(cdi->dictionary[0].value, 31);
	EXPECT_EQ(cdi->dictionary[1].name, "id1");
	EXPECT_EQ(cdi->dictionary[1].value, 35);
	EXPECT_EQ(cdi->dictionary[2].name, "id2");
	EXPECT_EQ(cdi->dictionary[2].value, 43);

	EXPECT_EQ(cdi->subClass->subString, "modified");
	cdi->subClass->subString = "modified again";

	cdi->stringValue = "This is a modified string";
    
	Ref<CDITest> testObject = CreateObject<CDITest>(nullptr, "TestObj", OF_Transient);
	EXPECT_EQ(testObject->floatValue, 42.21f);
	EXPECT_EQ(testObject->stringValue, "This is a modified string");
	EXPECT_EQ(testObject->boolValue, true);

	EXPECT_EQ(testObject->arrayValue.GetSize(), 3);
	EXPECT_EQ(testObject->arrayValue[0], "Entry0");
	EXPECT_EQ(testObject->arrayValue[1], "Entry1");
	EXPECT_EQ(testObject->arrayValue[2], "Entry2");

	EXPECT_EQ(testObject->dictionary.GetSize(), 3);
	EXPECT_EQ(testObject->dictionary[0].name, "id0");
	EXPECT_EQ(testObject->dictionary[0].value, 31);
	EXPECT_EQ(testObject->dictionary[1].name, "id1");
	EXPECT_EQ(testObject->dictionary[1].value, 35);
	EXPECT_EQ(testObject->dictionary[2].name, "id2");
	EXPECT_EQ(testObject->dictionary[2].value, 43);

	EXPECT_NE(cdi->subClass, testObject->subClass);
	EXPECT_EQ(testObject->subClass->GetOuter(), testObject);
	EXPECT_EQ(testObject->subClass->subString, "modified again");
	EXPECT_NE(testObject->subClass, cdi->subClass); // sub objects should always be deep-copied

	testObject->BeginDestroy();
    
	CE_DEREGISTER_TYPES(CDITest, CDIStruct, CDISubClass);
    TEST_END;
}


TEST(Object, CDI2)
{
	using namespace CDITests;

	TEST_BEGIN;
	CE_REGISTER_TYPES(TestObject, AnotherObject, TestStruct);

	auto anotherCDI = GetMutableDefaults<AnotherObject>();
	auto testCDI = GetMutableDefaults<TestObject>();

	// 1. CDI Test
	{
		EXPECT_EQ(testCDI->subobject->myString, "modified from TestObject");
		EXPECT_EQ(testCDI->subobject->data.stringArray.GetSize(), 2);
		EXPECT_EQ(testCDI->subobject->data.stringArray[0], "test0");
		EXPECT_EQ(testCDI->subobject->data.stringArray[1], "test1");
		EXPECT_EQ(testCDI->subobject->data.another, nullptr);
		EXPECT_EQ(testCDI->another, nullptr);

		EXPECT_EQ(anotherCDI->data.stringArray.GetSize(), 3);
		EXPECT_EQ(anotherCDI->data.stringArray[0], "another0");
		EXPECT_EQ(anotherCDI->data.stringArray[1], "another1");
		EXPECT_EQ(anotherCDI->data.stringArray[2], "another2");
		EXPECT_EQ(anotherCDI->data.another, anotherCDI);

		EXPECT_EQ(anotherCDI->myString, "default");

		testCDI->subobject->myString = "modified from CDI";
		testCDI->subobject->data.another = testCDI->subobject;
		testCDI->subobject->data.nonSerializedValue = 123;

		testCDI->transient = ModuleManager::Get().GetLoadedModuleTransientBundle("Core");
	}
	
	// 2. Object instantiation
	{
		Object* transient = CreateObject<Object>(nullptr, "Transient", OF_Transient);

		TestObject* test = CreateObject<TestObject>(nullptr, "TestObject");
		EXPECT_EQ(test->GetOuter(), nullptr);
		EXPECT_EQ(test->subobject->myString, "modified from CDI");
		EXPECT_EQ(test->subobject->data.nonSerializedValue, 123);
		EXPECT_EQ(test->subobject->data.stringArray.GetSize(), 2);
		EXPECT_EQ(test->subobject->data.stringArray[0], "test0");
		EXPECT_EQ(test->subobject->data.stringArray[1], "test1");
		EXPECT_EQ(test->transient, testCDI->transient);
        EXPECT_EQ(test->transient, ModuleManager::Get().GetLoadedModuleTransientBundle("Core"));
		EXPECT_EQ(test->subobject->data.another, test->subobject);
		EXPECT_NE(test->another, nullptr);

		anotherCDI->test = test;
        anotherCDI->myString = "modified anotherCDI";

		AnotherObject* another = CreateObject<AnotherObject>(nullptr, "AnotherObject");
		EXPECT_EQ(another->GetOuter(), nullptr);
		EXPECT_EQ(another->test, test);
		EXPECT_EQ(another->myString, "modified anotherCDI");
		EXPECT_EQ(another->data.another, another);

		test->BeginDestroy();
		another->BeginDestroy();
		transient->BeginDestroy();
	}

	CE_DEREGISTER_TYPES(TestObject, AnotherObject, TestStruct);
	TEST_END;
}

TEST(Object, Events)
{
	TEST_BEGIN;
	CERegisterModuleTypes();
	CE_REGISTER_TYPES(
		EventTests::SenderClass,
		EventTests::ReceiverClass
	);

	{
		int value = 0;

		ScriptDelegate<void(const String& string)> test = [&](const String& string)
			{
				String::TryParse(string, value);
			};

		test("412");

		EXPECT_EQ(value, 412);
	}

	{
		using namespace EventTests;

		Ref<SenderClass> sender = CreateObject<SenderClass>(nullptr, "Sender");
		Ref<ReceiverClass> receiver = CreateObject<ReceiverClass>(nullptr, "Receiver");
		EXPECT_EQ(receiver->text, "");
		EXPECT_EQ(receiver->object, nullptr);
		EXPECT_EQ(receiver->fileAction, IO::FileAction::Add);

		IO::FileAction fileAction = IO::FileAction::Add;

		// - Bindings -

		sender->onTextChanged += FUNCTION_BINDING(receiver, OnTextChangedCallback);

		IScriptEvent* onObjectEvent = sender->GetClass()->FindField("onObjectEvent")->GetFieldEventValue(sender.Get());
		onObjectEvent->Bind(FUNCTION_BINDING(receiver, OnObjectEvent));

		auto lambda = [&fileAction](CE::IO::FileAction newFileAction)
			{
				fileAction = newFileAction;
			};

		int lambda2Called = 0;

		auto lambda2 = [&]
			{
				lambda2Called++;
			};

		DelegateHandle fileActionHandle = sender->fileActionEvent + lambda;
		DelegateHandle lambda2Handle = sender->fileActionEvent + lambda2;

		// - Broadcasts -

		sender->onTextChanged("Hello World");
		EXPECT_EQ(receiver->text, "Hello World");

		sender->onObjectEvent.Broadcast(receiver.Get());
		EXPECT_EQ(receiver->object, receiver);

		onObjectEvent->Broadcast({ sender.Get() });
		EXPECT_EQ(receiver->object, sender);

		EXPECT_EQ(lambda2Called, 0);
		sender->fileActionEvent.Broadcast(IO::FileAction::Modified);
		EXPECT_EQ(fileAction, IO::FileAction::Modified);
		EXPECT_EQ(lambda2Called, 1);

		sender->onTextChanged -= FUNCTION_BINDING(receiver, OnTextChangedCallback);
		sender->onTextChanged.Broadcast("New Text");
		EXPECT_EQ(receiver->text, "Hello World");

		sender->onTextChanged += FUNCTION_BINDING(receiver, OnTextChangedCallback);
		sender->onTextChanged.Broadcast("ReAdded");
		EXPECT_EQ(receiver->text, "ReAdded");

		receiver->BeginDestroy();

		// To make sure this doesn't crash because we have destroyed the receiver object
		sender->onTextChanged.Broadcast("New Text");
		onObjectEvent->Broadcast({ sender.Get() });
		sender->fileActionEvent.Broadcast(IO::FileAction::Delete);
		EXPECT_EQ(fileAction, IO::FileAction::Delete);

		// - Unbinding -

		EXPECT_EQ(lambda2Called, 2);

		sender->fileActionEvent -= lambda2Handle;
		sender->fileActionEvent -= fileActionHandle;
		sender->fileActionEvent.Broadcast(IO::FileAction::Moved);

		EXPECT_EQ(fileAction, IO::FileAction::Delete);
		EXPECT_EQ(lambda2Called, 2);

		sender->BeginDestroy();
	}

	CE_DEREGISTER_TYPES(
		EventTests::SenderClass,
		EventTests::ReceiverClass
	);
	CEDeregisterModuleTypes();
	TEST_END;
}

#pragma endregion

/**********************************************
*   JSON
*/

#pragma region JSON

const char JSON_Writer_Test1_Comparison[] = R"({
	"prop1": 4212.212,
	"name": "Some Name",
	"child": 
	{
		"float": 12.21,
		"name": "Child Name",
		"child_array": 
		[
			"My String",
			-1244,
			{
				"float": 12.21,
				"name": "Child Name",
				"int1": 41224,
				"int2": -41224
			},
			123.123
		],
		"int1": 41224,
		"int2": -41224
	},
	"prop2": 52635
})";

const char JSON_Writer_Test2_Comparison[] = R"([
	{
		"some_array": 
		[
			"child0",
			123,
			42.212,
			false
		],
		"name": "Some name"
	},
	"item0",
	42.212
])";

const char JSON_Writer_Test2_Comment_Comparison[] = R"([
	{
		// this is a comment
		"some_array":
		[
			"child0", // another comment
			123,
			42.212,
			false// a bad comment
		],
		"name": "Some name"// another bad comment
	},
	"item0",
	42.212
])";

TEST(JSON, Writer)
{
    TEST_BEGIN;

    auto stream = MemoryStream(2048);
    stream.SetAsciiMode(true);

    // 1. Object Start
    auto writer = PrettyJsonWriter::Create(&stream);

    if (writer.WriteObjectStart())
    {
        writer.WriteValue("prop1", 4212.212f);
        writer.WriteValue("name", "Some Name");

        if (writer.WriteObjectStart("child"))
        {
            writer.WriteValue("float", 12.21f);
            writer.WriteValue("name", "Child Name");

            if (writer.WriteArrayStart("child_array"))
            {
                writer.WriteValue("My String");
                writer.WriteValue(-1244);
                if (writer.WriteObjectStart())
                {
                    writer.WriteValue("float", 12.21f);
                    writer.WriteValue("name", "Child Name");
                    writer.WriteValue<u32>("int1", 41224);
                    writer.WriteValue<s32>("int2", -41224);

                    writer.WriteObjectClose();
                }
                writer.WriteValue(123.123f);

                writer.WriteArrayClose();
            }

            writer.WriteValue<u32>("int1", 41224);
            writer.WriteValue<s32>("int2", -41224);

            writer.WriteObjectClose();
        }
        
        writer.WriteValue("prop2", 52635);

        writer.WriteObjectClose();
    }
    
    stream.Write('\0');
    auto comp = std::strcmp((char*)stream.GetRawDataPtr(), JSON_Writer_Test1_Comparison);
    EXPECT_EQ(comp, 0);

    // 2. Array Start
    stream.Seek(0);
    writer = PrettyJsonWriter::Create(&stream);

    if (writer.WriteArrayStart())
    {
        if (writer.WriteObjectStart())
        {
            if (writer.WriteArrayStart("some_array"))
            {
                writer.WriteValue("child0");
                writer.WriteValue(123);
                writer.WriteValue(42.212f);
                writer.WriteValue(false);
                
                writer.WriteArrayClose();
            }
            writer.WriteValue("name", "Some name");
            
            writer.WriteObjectClose();
        }
        writer.WriteValue("item0");
        writer.WriteValue(42.212f);
        
        writer.WriteArrayClose();
    }

    stream.Write('\0');
    comp = std::strcmp((char*)stream.GetRawDataPtr(), JSON_Writer_Test2_Comparison);
    EXPECT_EQ(comp, 0);

    stream.Close();
    
    TEST_END;
}

TEST(JSON, TokenParser)
{
    TEST_BEGIN;

    auto stream = MemoryStream((void*)JSON_Writer_Test2_Comment_Comparison, COUNTOF(JSON_Writer_Test2_Comment_Comparison), Stream::Permissions::ReadOnly);
    stream.SetAsciiMode(true);

    struct TokenPair
    {
        JsonToken token;
        String lexeme;
    };
    
    JsonReader reader = JsonReader::Create(&stream);
    
    Array<TokenPair> tokenArray{};

    while (reader.NextAvailable())
    {
        JsonToken token = JsonToken::None;
        String lexeme = "";
        auto value = reader.ParseNextToken(token, lexeme);
        if (token != JsonToken::None)
        {
            tokenArray.Add({token, lexeme});
        }
    }

    EXPECT_EQ(tokenArray.GetSize(), 23);
    EXPECT_EQ(tokenArray[0].token, JsonToken::SquareOpen);
    {
        EXPECT_EQ(tokenArray[1].token, JsonToken::CurlyOpen);
        {
            EXPECT_EQ(tokenArray[2].token, JsonToken::Identifier); EXPECT_EQ(tokenArray[2].lexeme, "some_array");
            EXPECT_EQ(tokenArray[3].token, JsonToken::Colon);
        
            EXPECT_EQ(tokenArray[4].token, JsonToken::SquareOpen);
            {
                EXPECT_EQ(tokenArray[5].token, JsonToken::String); EXPECT_EQ(tokenArray[5].lexeme, "child0");
                EXPECT_EQ(tokenArray[6].token, JsonToken::Comma);
                EXPECT_EQ(tokenArray[7].token, JsonToken::Number); EXPECT_EQ(tokenArray[7].lexeme, "123");
                EXPECT_EQ(tokenArray[8].token, JsonToken::Comma);
                EXPECT_EQ(tokenArray[9].token, JsonToken::Number); EXPECT_EQ(tokenArray[9].lexeme, "42.212");
                EXPECT_EQ(tokenArray[10].token, JsonToken::Comma);
                EXPECT_EQ(tokenArray[11].token, JsonToken::False);
            }
            EXPECT_EQ(tokenArray[12].token, JsonToken::SquareClose);

            EXPECT_EQ(tokenArray[13].token, JsonToken::Comma);

            EXPECT_EQ(tokenArray[14].token, JsonToken::Identifier); EXPECT_EQ(tokenArray[14].lexeme, "name");
            EXPECT_EQ(tokenArray[15].token, JsonToken::Colon);
            EXPECT_EQ(tokenArray[16].token, JsonToken::String); EXPECT_EQ(tokenArray[16].lexeme, "Some name");
        }
        EXPECT_EQ(tokenArray[17].token, JsonToken::CurlyClose);

        EXPECT_EQ(tokenArray[18].token, JsonToken::Comma);

        EXPECT_EQ(tokenArray[19].token, JsonToken::String); EXPECT_EQ(tokenArray[19].lexeme, "item0");

        EXPECT_EQ(tokenArray[20].token, JsonToken::Comma);

        EXPECT_EQ(tokenArray[21].token, JsonToken::Number); EXPECT_EQ(tokenArray[21].lexeme, "42.212");
    }
    EXPECT_EQ(tokenArray[22].token, JsonToken::SquareClose);
    
    TEST_END;
}

TEST(JSON, Serializer)
{
    TEST_BEGIN;

    auto stream = MemoryStream((void*)JSON_Writer_Test2_Comparison, COUNTOF(JSON_Writer_Test2_Comparison), Stream::Permissions::ReadOnly);
    stream.SetAsciiMode(true);

    // 1. Deserialize JSON_Writer_Test2_Comparison

    auto root = JsonSerializer::Deserialize(&stream);
    auto& rootRef = *root;
    stream.Close();

    EXPECT_TRUE(rootRef.IsArrayValue());
    EXPECT_EQ(rootRef.GetSize(), 3);
    {
        EXPECT_TRUE(rootRef[0].IsObjectValue());
        {
            EXPECT_EQ(rootRef[0].GetSize(), 2);
            EXPECT_TRUE(rootRef[0].KeyExists("some_array"));
            EXPECT_TRUE(rootRef[0].KeyExists("name"));
        
            EXPECT_TRUE(rootRef[0]["some_array"].IsArrayValue());
            {
                EXPECT_EQ(rootRef[0]["some_array"].GetSize(), 4);
                EXPECT_TRUE(rootRef[0]["some_array"][0].IsStringValue());
                EXPECT_EQ(rootRef[0]["some_array"][0].GetStringValue(), "child0");
                EXPECT_TRUE(rootRef[0]["some_array"][1].IsNumberValue());
                EXPECT_EQ(rootRef[0]["some_array"][1].GetNumberValue(), 123);
                EXPECT_TRUE(rootRef[0]["some_array"][2].IsNumberValue());
                EXPECT_EQ(rootRef[0]["some_array"][2].GetNumberValue(), 42.212);
                EXPECT_TRUE(rootRef[0]["some_array"][3].IsBoolValue());
                EXPECT_EQ(rootRef[0]["some_array"][3].GetBoolValue(), false);
            }

            EXPECT_TRUE(rootRef[0]["name"].IsStringValue());
            EXPECT_EQ(rootRef[0]["name"].GetStringValue(), "Some name");
        }

        EXPECT_TRUE(rootRef[1].IsStringValue());
        EXPECT_EQ(rootRef[1].GetStringValue(), "item0");
        EXPECT_TRUE(rootRef[2].IsNumberValue());
        EXPECT_EQ(rootRef[2].GetNumberValue(), 42.212);
    }

	// 2. Serialize JSON_Writer_Test2_Comparison

	auto copyStream = MemoryStream(2048);
	copyStream.SetAsciiMode(true);

	JsonSerializer::Serialize(&copyStream, root);
	// Order can be different
	//EXPECT_EQ(strcmp(JSON_Writer_Test2_Comparison, (char*)copyStream.GetRawDataPtr()), 0);

	copyStream.Close();

	delete root;

	// 3. Deserialize2 method

	auto stream2 = MemoryStream((void*)JSON_Writer_Test2_Comparison, COUNTOF(JSON_Writer_Test2_Comparison), Stream::Permissions::ReadOnly);
	stream2.SetAsciiMode(true);

	JValue json = nullptr;
	JsonSerializer::Deserialize2(&stream2, json);
	stream2.Close();

	EXPECT_TRUE(json.IsArrayValue());
	EXPECT_EQ(json.GetSize(), 3);
	{
		EXPECT_TRUE(json[0].IsObjectValue());
		{
			EXPECT_EQ(json[0].GetSize(), 2);
			EXPECT_TRUE(json[0].KeyExists("some_array"));
			EXPECT_TRUE(json[0].KeyExists("name"));

			EXPECT_TRUE(json[0]["some_array"].IsArrayValue());
			{
				EXPECT_EQ(json[0]["some_array"].GetSize(), 4);
				EXPECT_TRUE(json[0]["some_array"][0].IsStringValue());
				EXPECT_EQ(json[0]["some_array"][0].GetStringValue(), "child0");
				EXPECT_TRUE(json[0]["some_array"][1].IsNumberValue());
				EXPECT_EQ(json[0]["some_array"][1].GetNumberValue(), 123);
				EXPECT_TRUE(json[0]["some_array"][2].IsNumberValue());
				EXPECT_EQ(json[0]["some_array"][2].GetNumberValue(), 42.212);
				EXPECT_TRUE(json[0]["some_array"][3].IsBoolValue());
				EXPECT_EQ(json[0]["some_array"][3].GetBoolValue(), false);
			}

			EXPECT_TRUE(json[0]["name"].IsStringValue());
			EXPECT_EQ(json[0]["name"].GetStringValue(), "Some name");
		}

		EXPECT_TRUE(json[1].IsStringValue());
		EXPECT_EQ(json[1].GetStringValue(), "item0");
		EXPECT_TRUE(json[2].IsNumberValue());
		EXPECT_EQ(json[2].GetNumberValue(), 42.212);
	}
    
	// 4. Serialize2 method

	auto copyStream2 = MemoryStream(2048);
	copyStream2.SetAsciiMode(true);

	JsonSerializer::Serialize2(&copyStream2, json);

	copyStream2.Close();

    TEST_END;
}

TEST(JSON, FieldSerializer)
{
	TEST_BEGIN;
	CERegisterModuleTypes();

	using namespace JsonTests;

	MemoryStream stream{ 1024 };

	{
		SerializedData data{};
		data.strVal = "My String";
		data.strArray = { "Item0", "Item1", "Item2", "Item3" };
		data.inner.value = "Inner struct";
		data.inner.myInt = 412;
		data.inner.vecValue.left = 0;
		data.inner.vecValue.right = 1;
		data.inner.vecValue.top = 2;
		data.inner.vecValue.bottom = 3;
		data.innerArray = Array<InnerStruct>{ { "i0", 120 }, { "i1", 121 }, { "i2", 122 }, { "i3", 123 }, { "i4", 124 } };
		data.innerArray[2].nextInner = Array<InnerStruct>{ { "c0", 0 }, { "c1", 1 }, { "c2", 2 }, { "c3", 3 } };

		JsonFieldSerializer fieldSerializer{ SerializedData::Type(), &data };

		while (fieldSerializer.HasNext())
		{
			fieldSerializer.WriteNext(&stream);
		}
	}

	stream.Write('\0');
	stream.Seek(0);

	{
		SerializedData data{};

		JsonFieldDeserializer fieldDeserializer{ SerializedData::Type(), &data };

		fieldDeserializer.Deserialize(&stream);

		EXPECT_EQ(data.strVal, "My String");
		EXPECT_EQ(data.strArray.GetSize(), 4);
		{
			EXPECT_EQ(data.strArray[0], "Item0");
			EXPECT_EQ(data.strArray[1], "Item1");
			EXPECT_EQ(data.strArray[2], "Item2");
			EXPECT_EQ(data.strArray[3], "Item3");
		}
		EXPECT_EQ(data.inner.value, "Inner struct");
		EXPECT_EQ(data.inner.myInt, 412);

		{
			EXPECT_EQ(data.inner.vecValue.left, 0);
			EXPECT_EQ(data.inner.vecValue.right, 1);
			EXPECT_EQ(data.inner.vecValue.top, 2);
			EXPECT_EQ(data.inner.vecValue.bottom, 3);
		}

		EXPECT_EQ(data.innerArray.GetSize(), 5);
		{
			EXPECT_EQ(data.innerArray[0].value, "i0");
			EXPECT_EQ(data.innerArray[0].myInt, 120);
			EXPECT_EQ(data.innerArray[1].value, "i1");
			EXPECT_EQ(data.innerArray[1].myInt, 121);
			EXPECT_EQ(data.innerArray[2].value, "i2");
			EXPECT_EQ(data.innerArray[2].myInt, 122);
			{
				EXPECT_EQ(data.innerArray[2].nextInner.GetSize(), 4);
			}
			EXPECT_EQ(data.innerArray[3].value, "i3");
			EXPECT_EQ(data.innerArray[3].myInt, 123);
			EXPECT_EQ(data.innerArray[4].value, "i4");
			EXPECT_EQ(data.innerArray[4].myInt, 124);
		}
	}

	stream.Close();

	{
		JValue root = JObject();
		root["vecValue"] = JObject();
		root["vecValue"]["left"] = 0;
		root["vecValue"]["right"] = 1;
		root["vecValue"]["top"] = 2;
		root["vecValue"]["bottom"] = 3;

		InnerStruct inner{};

		JsonFieldDeserializer fieldDeserializer{ InnerStruct::Type(), &inner };

		while (fieldDeserializer.HasNext())
		{
			fieldDeserializer.ReadNext(root);
		}

		EXPECT_EQ(inner.vecValue.left, 0);
		EXPECT_EQ(inner.vecValue.right, 1);
		EXPECT_EQ(inner.vecValue.top, 2);
		EXPECT_EQ(inner.vecValue.bottom, 3);
	}

	CEDeregisterModuleTypes();
	TEST_END;
}

class JsonTestPrefs : public CE::BasePrefs
{
public:
	typedef CE::BasePrefs Super;

	JsonTestPrefs()
	{}

	void LoadPrefs() override
	{
		auto path = PlatformDirectories::GetLaunchDir() / "TestPrefs.json";
		Super::LoadPrefs(path);
	}

	void SavePrefs() override
	{
		auto path = PlatformDirectories::GetLaunchDir() / "TestPrefs.json";
		Super::SavePrefs(path);
	}
};

struct JsonTestStruct
{
	CE_STRUCT(JsonTestStruct)
public:

	String stringValue{};

	f32 numberValue = 0;

	b8 boolValue = false;

	Array<String> stringArray{};

	Array<JsonTestStruct> subarray{};
};

CE_RTTI_STRUCT(, , JsonTestStruct,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(stringValue)
		CE_FIELD(numberValue)
		CE_FIELD(boolValue)
		CE_FIELD(stringArray)
		CE_FIELD(subarray)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(, , JsonTestStruct)

TEST(JSON, Prefs)
{
	TEST_BEGIN;
	CE_REGISTER_TYPES(JsonTestStruct);

	auto path = PlatformDirectories::GetLaunchDir() / "TestPrefs.json";
	
	{
		JsonTestStruct jsonStruct{};
		jsonStruct.stringValue = "This is a string";
		jsonStruct.numberValue = 12.42f;
		jsonStruct.boolValue = true;
		jsonStruct.stringArray = { "item0", "item1", "item2", "item3" };

		jsonStruct.subarray.Resize(1);
		jsonStruct.subarray[0].stringValue = "child0";
		jsonStruct.subarray[0].numberValue = 3;
		jsonStruct.subarray[0].boolValue = false;
		jsonStruct.subarray[0].stringArray = { "entry0", "entry1", "entry2" };
		jsonStruct.subarray[0].subarray = {};

		JsonTestPrefs prefs{};
		prefs.SetStruct("testStruct", &jsonStruct);

		prefs.SetBool("boolValue", true);
		prefs.SetString("stringValue", "My String");
		prefs.SetFloat("floatValue", 12.12f);
		prefs.SetNull("nullValue");

		prefs.SavePrefs();
	}

	{
		JsonTestStruct jsonStruct{};
		
		JsonTestPrefs prefs{};
		prefs.LoadPrefs();

		EXPECT_EQ(prefs.GetString("stringValue"), "My String");
		EXPECT_EQ(prefs.GetBool("boolValue"), true);
		EXPECT_EQ(prefs.IsNullValue("nullValue"), true);
		EXPECT_EQ(prefs.GetFloat("floatValue"), 12.12f);

		prefs.GetStruct("testStruct", &jsonStruct);
		EXPECT_EQ(jsonStruct.stringValue, "This is a string");
		EXPECT_EQ(jsonStruct.numberValue, 12.42f);
		EXPECT_EQ(jsonStruct.boolValue, true);
		EXPECT_EQ(jsonStruct.stringArray.GetSize(), 4);
		for (int i = 0; i < 4; i++)
		{
			EXPECT_EQ(jsonStruct.stringArray[i], String::Format("item{}", i));
		}

		EXPECT_EQ(jsonStruct.subarray.GetSize(), 1);
		EXPECT_EQ(jsonStruct.subarray[0].stringValue, "child0");
		EXPECT_EQ(jsonStruct.subarray[0].numberValue, 3);
		EXPECT_EQ(jsonStruct.subarray[0].boolValue, false);
		EXPECT_EQ(jsonStruct.subarray[0].stringArray.GetSize(), 3);
		for (int i = 0; i < 3; i++)
		{
			EXPECT_EQ(jsonStruct.subarray[0].stringArray[i], String::Format("entry{}", i));
		}

		prefs.DeletePrefs();
	}

	IO::Path::Remove(path);

	CE_DEREGISTER_TYPES(JsonTestStruct);
	TEST_END;
}


TEST(JSON, Manipulation)
{
	TEST_BEGIN;

	JValue root = JObject();
	root["item0"] = 12.12f;
	root["item1"] = "String Value";
	root["item2"] = true;

	root["array"] = JArray();
	root["array"][0] = 123;
	root["array"][1] = "Item String";

	root["child"] = JObject();
	root["child"]["number"] = 43.21f;
	root["child"]["bool"] = true;
	root["child"]["string"] = "child string";

	JValue test = 12.42f;
	test = "String Value";
	test = JArray({ 12.42f, "string", true });
	test = JObject({ { "key0", true }, { "key1", "string" }});

	TEST_END;
}

#pragma endregion

/**********************************************
*   Serialization
*/

#pragma region Serialization

TEST(Serialization, BasicStreams)
{
    TEST_BEGIN;

    auto launchDir = PlatformDirectories::GetLaunchDir();

    // 1. Memory Stream (Binary)
    auto memBinStream = MemoryStream(256);
    memBinStream.SetBinaryMode(true);
    EXPECT_TRUE(memBinStream.IsOpen());
    
    memBinStream << "Hello";
    memBinStream << (s32)-521221;
    memBinStream << "New String";
    memBinStream << 12.425f;

    memBinStream.Seek(0);

    String str = "";
    s32 integer = 0;
    f32 single = 0;
    memBinStream >> str;
    EXPECT_EQ(str, "Hello");
    memBinStream >> integer;
    EXPECT_EQ(integer, -521221);
    memBinStream >> str;
    EXPECT_EQ(str, "New String");
    memBinStream >> single;
    EXPECT_EQ(single, 12.425f);

    memBinStream.Close();
    EXPECT_FALSE(memBinStream.IsOpen());

    // 2. Memory Stream (Ascii)

    auto memAsciiStream = MemoryStream(256);
    memAsciiStream.SetAsciiMode(true);
    EXPECT_TRUE(memAsciiStream.IsOpen());

    memAsciiStream << "This is a String! Integer = ";
    memAsciiStream << 123;
    memAsciiStream << ";\n";
	memAsciiStream << 412 << "\n";
	memAsciiStream << 14.124f << "\n";
    memAsciiStream << "float = ";
    memAsciiStream << 12.452f;

    memAsciiStream.Seek(0);
	String line = ""; f32 floatVal = 0;
    memAsciiStream >> line;
    EXPECT_EQ(line, "This is a String! Integer = 123;");
	int number = 0;
	memAsciiStream >> number;
	EXPECT_EQ(number, 412);
	memAsciiStream >> floatVal;
	EXPECT_EQ(floatVal, 14.124f);
    memAsciiStream >> line;
    EXPECT_EQ(line, "float = 12.452");
    EXPECT_TRUE(memAsciiStream.IsOutOfBounds());
    
    memAsciiStream.Close();
    EXPECT_FALSE(memAsciiStream.IsOpen());

    // 3. File Stream (Binary)

    // Write
    auto path = launchDir / "TestFile.bin";
    if (path.Exists())
        IO::Path::Remove(path);
    
    auto fileBinStream = FileStream(path,Stream::Permissions::ReadWrite);
    fileBinStream.SetBinaryMode(true);
    EXPECT_TRUE(fileBinStream.IsOpen());

    fileBinStream << "This is a string";
    fileBinStream << (u8)250;
    fileBinStream << 41212.42f;
    fileBinStream << "New String";
	auto uuid = CE::Uuid();
	fileBinStream << uuid;
    
    fileBinStream.Close();
    EXPECT_FALSE(fileBinStream.IsOpen());

    // Read
    auto newFileBinStream = FileStream(path, Stream::Permissions::ReadOnly);
    newFileBinStream.SetBinaryMode(true);
    EXPECT_TRUE(newFileBinStream.IsOpen());

	str = ""; u8 byte = 0; single = 0; u64 longNum = 0;
    newFileBinStream >> str;
    EXPECT_EQ(str, "This is a string");
    newFileBinStream >> byte;
    EXPECT_EQ(byte, 250);
    newFileBinStream >> single;
    EXPECT_EQ(single, 41212.42f);
    newFileBinStream >> str;
    EXPECT_EQ(str, "New String");
	newFileBinStream >> longNum;
	//EXPECT_EQ(longNum, (u64)0xfe0000efac0000ca);
    //EXPECT_TRUE(newFileBinStream.IsOutOfBounds());

    newFileBinStream.Close();
    EXPECT_FALSE(newFileBinStream.IsOpen());
    
    IO::Path::Remove(path);

    // 4. File Stream (Ascii)
    auto fileAsciiStream = FileStream(path, Stream::Permissions::ReadWrite);
    fileAsciiStream.SetAsciiMode(true);
    EXPECT_TRUE(fileAsciiStream.IsOpen());

    fileAsciiStream << "This is an ascii file!";
    fileAsciiStream << " A new sentence\n";
    fileAsciiStream << "1234;\r\n";
    fileAsciiStream << "Last line!";

    fileAsciiStream.Close();
    EXPECT_FALSE(fileAsciiStream.IsOpen());

    auto newFileAsciiStream = FileStream(path, Stream::Permissions::ReadOnly);
    newFileAsciiStream.SetAsciiMode(true);
    EXPECT_TRUE(newFileAsciiStream.IsOpen());

    str = "";
    newFileAsciiStream >> str;
    EXPECT_EQ(str, "This is an ascii file! A new sentence");
    newFileAsciiStream >> str;
    EXPECT_EQ(str, "1234;");
    newFileAsciiStream >> str;
    EXPECT_EQ(str, "Last line!");
    EXPECT_TRUE(newFileAsciiStream.IsOutOfBounds());
    
    newFileAsciiStream.Close();
    EXPECT_FALSE(newFileAsciiStream.IsOpen());

    IO::Path::Remove(path);

    TEST_END;
}

const char Serialization_StructuredStream_Test_Json[] = R"([
	{
		"some_array": 
		[
			"child0",
			123,
			42.212,
			false
		],
		"name": "Some name"
	},
	"item0",
	42.212
])";

class BinaryBlobTest : public Object
{
	CE_CLASS(BinaryBlobTest, Object)
public:

	~BinaryBlobTest()
	{
		blob.Free();
	}

	String stringField = "default";

	BinaryBlob blob{};
};

CE_RTTI_CLASS(,,BinaryBlobTest,
	CE_SUPER(CE::Object),
	CE_NOT_ABSTRACT,
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(stringField)
		CE_FIELD(blob)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_CLASS_IMPL(,,BinaryBlobTest)

TEST(Serialization, BinaryBlob)
{
    TEST_BEGIN;
	CE_REGISTER_TYPES(BinaryBlobTest);
    
    // 1. Basic Blob test
    {
		MemoryStream stream = MemoryStream(1024);
        const char data[] = "1234567890";

		{
			BinaryBlob blob{};
			blob.LoadData((void*)data, COUNTOF(data));
			stream.SetBinaryMode(true);
			stream << blob;

			u8* dataPtr = (u8*)stream.GetRawDataPtr();
			s64 size = *(s64*)dataPtr;

			EXPECT_EQ(size, 11);
			for (int i = 0; i < 9; i++)
			{
				EXPECT_EQ(*(dataPtr + 8 + 8 + i), '1' + i);
			}
			EXPECT_EQ(*(dataPtr + 8 + 8 + 9), '0');

			blob.Free();
		}

		{
			stream.Seek(0);

			BinaryBlob blob{};
			
			stream >> blob;

			u8* dataPtr = (u8*)blob.GetDataPtr();
			s64 size = blob.GetDataSize();

			EXPECT_EQ(size, 11);
			for (int i = 0; i < 9; i++)
			{
				EXPECT_EQ(*(dataPtr + i), '1' + i);
			}
			EXPECT_EQ(*(dataPtr + 9), '0');

			blob.Free();
		}
        
        stream.Close();
    }

	CE_DEREGISTER_TYPES(BinaryBlobTest);
    TEST_END;
}

namespace SerializationTests {
	struct MyData
	{
		CE_STRUCT(MyData)
	public:

		CE::String string = "default";

		Vec4 vector = {};

		ClassType* clazz = nullptr;

		Array<String> array{};
	};

	class TestClass1 : public Object
	{
		CE_CLASS(TestClass1, Object)
	public:

		TestClass1()
		{
			myString = "default";
		}

		String myString = "";

		Array<MyData> dataList{};

	};
}

CE_RTTI_STRUCT(,SerializationTests, MyData,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(string)
		CE_FIELD(vector)
		CE_FIELD(clazz)
		CE_FIELD(array)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(,SerializationTests, MyData)

CE_RTTI_CLASS(,SerializationTests, TestClass1,
	CE_SUPER(CE::Object),
	CE_NOT_ABSTRACT,
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(myString)
		CE_FIELD(dataList)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_CLASS_IMPL(,SerializationTests, TestClass1)

struct TestPrefsStruct
{
	CE_STRUCT(TestPrefsStruct)
public:

	Array<Name> nameList{};

	Color colorValue{};
};

CE_RTTI_STRUCT(,,TestPrefsStruct,
	CE_SUPER(),
	CE_ATTRIBS(),
	CE_FIELD_LIST(
		CE_FIELD(nameList)
		CE_FIELD(colorValue)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_STRUCT_IMPL(,,TestPrefsStruct)

class TestPrefsClass : public Object
{
	CE_CLASS(TestPrefsClass, Object)
public:

	Array<String> stringList{};

	TestPrefsStruct prefsStruct{};
};

CE_RTTI_CLASS(,,TestPrefsClass,
	CE_SUPER(Object),
	CE_NOT_ABSTRACT,
	CE_ATTRIBS(Prefs = Test),
	CE_FIELD_LIST(
		CE_FIELD(stringList, Prefs)
		CE_FIELD(prefsStruct, Prefs)
	),
	CE_FUNCTION_LIST()
)
CE_RTTI_CLASS_IMPL(,,TestPrefsClass)

TEST(Serialization, Prefs)	
{
	TEST_BEGIN;
	CE_REGISTER_TYPES(TestPrefsStruct);
	CE_REGISTER_TYPES(TestPrefsClass);
	Prefs::Get().LoadPrefsJson();
	{
		TestPrefsClass* prefsObject = CreateObject<TestPrefsClass>(nullptr, "TestObject");
		prefsObject->stringList.Clear();
		prefsObject->stringList.AddRange({ "item0", "item1", "item2", "item3" });

		prefsObject->prefsStruct.nameList.Clear();
		prefsObject->prefsStruct.nameList.AddRange({ "name0", "name1" });
		prefsObject->prefsStruct.colorValue = Color::RGBA(128, 64, 32);

		prefsObject->BeginDestroy();
	}
	Prefs::Get().SavePrefsJson();

	Prefs::Get().LoadPrefsJson();
	{
		TestPrefsClass* prefsObject = CreateObject<TestPrefsClass>(nullptr, "TestObject");
		EXPECT_EQ(prefsObject->stringList.GetSize(), 4);
		for (int i = 0; i < 4; ++i)
		{
			EXPECT_EQ(prefsObject->stringList[i], String::Format("item{}", i));
		}

		EXPECT_EQ(prefsObject->prefsStruct.nameList.GetSize(), 2);
		for (int i = 0; i < 2; ++i)
		{
			EXPECT_EQ(prefsObject->prefsStruct.nameList[i], String::Format("name{}", i));
		}

		EXPECT_EQ(prefsObject->prefsStruct.colorValue.ToU32(), Color::RGBA(128, 64, 32).ToU32());

		prefsObject->BeginDestroy();
	}
	CE_DEREGISTER_TYPES(TestPrefsClass, TestPrefsStruct);
	TEST_END;
}

#pragma endregion


/**********************************************
*   Delegates
*/

#pragma region Delegates

class DelegateTestClass
{
public:
    DelegateTestClass()
    {

    }
    ~DelegateTestClass()
    {

    }

    static void TestFunc1(String& str)
    {
        str += ",TestFunc1";
    }

    static void TestFunc2(String& str)
    {
        str += ",TestFunc2";
    }

    void TestFunc3(String& str)
    {
        str += ",TestFunc3";
    }
};

/// Single Cast Delegates

TEST(Delegates, SingleCast)
{
    TEST_BEGIN;

    Delegate<void(String&)> delegate = &DelegateTestClass::TestFunc1;

    String str = "SingleCast";
    delegate(str);

    EXPECT_EQ(str, "SingleCast,TestFunc1");

    delegate = &DelegateTestClass::TestFunc2;
    delegate(str);

    EXPECT_EQ(str, "SingleCast,TestFunc1,TestFunc2");

    DelegateTestClass inst{};

    delegate = MemberDelegate(&DelegateTestClass::TestFunc3, &inst);
    delegate(str);
    
    EXPECT_EQ(str, "SingleCast,TestFunc1,TestFunc2,TestFunc3");

    TEST_END;
}

/// Multi Cast Delegates

TEST(Delegates, MultiCast)
{
    TEST_BEGIN;

    MultiCastDelegate<void(String&)> multiCastDelegate{};

    String str = "MultiCast";
    auto func1 = multiCastDelegate.AddDelegateInstance(&DelegateTestClass::TestFunc1);
    auto func2 = multiCastDelegate.AddDelegateInstance(&DelegateTestClass::TestFunc2);
    auto func1Copy = multiCastDelegate.AddDelegateInstance(&DelegateTestClass::TestFunc1);
    multiCastDelegate.Broadcast(str);
    EXPECT_EQ(str, "MultiCast,TestFunc1,TestFunc2,TestFunc1");

    str = "MultiCast";
    multiCastDelegate.RemoveDelegateInstance(func1);
    multiCastDelegate.Broadcast(str);
    EXPECT_EQ(str, "MultiCast,TestFunc2,TestFunc1");

    str = "MultiCast";
    multiCastDelegate.RemoveDelegateInstance(func2);
    multiCastDelegate.Broadcast(str);
    EXPECT_EQ(str, "MultiCast,TestFunc1");

    str = "MultiCast";
    multiCastDelegate.ClearAll();
    multiCastDelegate.Broadcast(str);
    EXPECT_EQ(str, "MultiCast");

    str = "MultiCast";
    func1 = multiCastDelegate.AddDelegateInstance(&DelegateTestClass::TestFunc1);
    DelegateTestClass inst{};
    auto func3 = multiCastDelegate.AddDelegateInstance(MemberDelegate(&DelegateTestClass::TestFunc3, &inst));
    multiCastDelegate.Broadcast(str);
    multiCastDelegate.ClearAll();
    EXPECT_EQ(str, "MultiCast,TestFunc1,TestFunc3");

    TEST_END;
}

/// Module Load/Unload Events

String Core_Delegates_Modules_Message = "";

void Core_Delegates_OnModuleLoaded(ModuleInfo* info)
{
    Core_Delegates_Modules_Message = "Loaded:" + info->moduleName.GetString() + (info->isPlugin ? ",true" : ",false") + (info->isLoaded ? ",true" : ",false");
}

void Core_Delegates_OnModuleUnloaded(ModuleInfo* info)
{
    Core_Delegates_Modules_Message = "Unloaded:" + info->moduleName.GetString() + (info->isPlugin ? ",true" : ",false") + (info->isLoaded ? ",true" : ",false");
}

void Core_Delegates_OnModuleFailedToLoad(const String& moduleName, ModuleLoadResult result)
{
    Core_Delegates_Modules_Message = String::Format("Failed:{},{}", moduleName, (int)result);
}

int Core_Delegates_OnClassRegistered_Count = 0;

void Core_Delegates_OnClassRegistered(ClassType* classType)
{
    Core_Delegates_OnClassRegistered_Count++;
}

TEST(Delegates, ModuleCallbacks)
{
    Core_Delegates_Modules_Message = "";
    Core_Delegates_OnClassRegistered_Count = 0;

    auto id1 = CoreDelegates::onAfterModuleLoad.AddDelegateInstance(&Core_Delegates_OnModuleLoaded);
    auto id2 = CoreDelegates::onAfterModuleUnload.AddDelegateInstance(&Core_Delegates_OnModuleUnloaded);
    auto id3 = CoreDelegates::onModuleFailedToLoad.AddDelegateInstance(&Core_Delegates_OnModuleFailedToLoad);
    auto id4 = CoreObjectDelegates::onClassRegistered.AddDelegateInstance(&Core_Delegates_OnClassRegistered);

    EXPECT_FALSE(ModuleManager::Get().IsModuleLoaded("Core"));
    ModuleManager::Get().LoadModule("Core");
    EXPECT_TRUE(ModuleManager::Get().IsModuleLoaded("Core"));
    EXPECT_EQ(Core_Delegates_Modules_Message, "Loaded:Core,false,true");
    EXPECT_GT(Core_Delegates_OnClassRegistered_Count, 0); // > 0

    ModuleManager::Get().LoadModule("Core");
    EXPECT_EQ(Core_Delegates_Modules_Message, String::Format("Failed:Core,{}", (int)ModuleLoadResult::AlreadyLoaded));

    ModuleManager::Get().LoadModule("ModuleThatDoesNotExist");
    EXPECT_EQ(Core_Delegates_Modules_Message, String::Format("Failed:ModuleThatDoesNotExist,{}", (int)ModuleLoadResult::DllNotFound));

    EXPECT_TRUE(ModuleManager::Get().IsModuleLoaded("Core"));
    ModuleManager::Get().UnloadModule("Core");
    EXPECT_FALSE(ModuleManager::Get().IsModuleLoaded("Core"));
    EXPECT_EQ(Core_Delegates_Modules_Message, "Unloaded:Core,false,false");

    CoreDelegates::onAfterModuleLoad.RemoveDelegateInstance(id1);
    CoreDelegates::onAfterModuleUnload.RemoveDelegateInstance(id2);
    CoreDelegates::onModuleFailedToLoad.RemoveDelegateInstance(id3);
    CoreObjectDelegates::onClassRegistered.RemoveDelegateInstance(id4);

    EXPECT_TRUE(CoreDelegates::onAfterModuleLoad.GetInvocationListCount() == 0);
    EXPECT_TRUE(CoreDelegates::onAfterModuleUnload.GetInvocationListCount() == 0);
    EXPECT_TRUE(CoreDelegates::onModuleFailedToLoad.GetInvocationListCount() == 0);

    Core_Delegates_Modules_Message = "";
    Core_Delegates_OnClassRegistered_Count = 0;
}

#pragma endregion


/**********************************************
* Bundles
*/

#pragma region Bundle

TEST(Bundle, Basic)
{
    TEST_BEGIN;
    using namespace BundleTests;
    CERegisterModuleTypes();

	IO::Path bundlePath = PlatformDirectories::GetLaunchDir() / "BasicTestBundle.casset";

	// 1. Write
    {
	    Ref<Bundle> bundle = CreateObject<Bundle>(nullptr, "BasicTestBundle");

		Ref<WritingTestObj2> testObject = CreateObject<WritingTestObj2>(bundle.Get(), "TestObject2");
    	EXPECT_EQ(testObject->GetOuter(), bundle);

		testObject->testStruct.obj1Ptr = nullptr;
		testObject->objectArray.Add(bundle);
		testObject->value = 123;
		testObject->testStruct.obj1Ptr = nullptr;
		testObject->testStruct.owner = bundle;
		testObject->testStruct.stringValue = "string value";

		testObject->arrayOfStruct.Resize(2);
		testObject->arrayOfStruct[0].owner = testObject;
		testObject->arrayOfStruct[0].stringValue = "Item 0";
		testObject->arrayOfStruct[0].anotherBase.stringValue = "internal 0";
		testObject->arrayOfStruct[1].owner = bundle;
		testObject->arrayOfStruct[1].stringValue = "Item 1";
		testObject->arrayOfStruct[1].anotherBase.stringValue = "internal 1";

		Bundle::SaveToDisk(bundle, nullptr);
    }

	// 2. Read
    {
    	LoadBundleArgs args{};

	    Ref<Bundle> bundle = Bundle::LoadBundle(nullptr, "/BasicTestBundle", args);

    	EXPECT_EQ(bundle->GetSubObjectCount(), 1);

    	Ref<WritingTestObj2> testObject = (Ref<WritingTestObj2>)bundle->LoadObject("TestObject2");

    	EXPECT_EQ(testObject->GetSubObjectCount(), 0);
    	EXPECT_EQ(testObject->GetOuter(), bundle);

    	EXPECT_EQ(testObject->value, 123);
    	EXPECT_EQ(testObject->objectArray.GetSize(), 1);
    	EXPECT_EQ(testObject->objectArray[0], bundle);
    	EXPECT_EQ(testObject->testStruct.owner, bundle);
    	EXPECT_EQ(testObject->testStruct.stringValue, "string value");
    	EXPECT_EQ(testObject->testStruct.obj1Ptr, nullptr);

    	EXPECT_EQ(testObject->arrayOfStruct.GetSize(), 2);
    	EXPECT_EQ(testObject->arrayOfStruct[0].owner, testObject);
    	EXPECT_EQ(testObject->arrayOfStruct[0].stringValue, "Item 0");
    	EXPECT_EQ(testObject->arrayOfStruct[0].anotherBase.stringValue, "internal 0");
    	EXPECT_EQ(testObject->arrayOfStruct[1].owner, bundle);
    	EXPECT_EQ(testObject->arrayOfStruct[1].stringValue, "Item 1");
    	EXPECT_EQ(testObject->arrayOfStruct[1].anotherBase.stringValue, "internal 1");
    }

	if (bundlePath.Exists())
	{
		IO::Path::Remove(bundlePath);
	}
    
    CEDeregisterModuleTypes();
    TEST_END;
}

namespace BundleTests
{
	class TestBundleResolver : public IBundleResolver
	{
	public:

		Name ResolveBundlePath(const Uuid& uuid) override
		{
			return bundlePathsByUuid[uuid];
		}

		HashMap<Uuid, Name> bundlePathsByUuid;
	};
}

TEST(Bundle, Multiple)
{
	TEST_BEGIN;
	using namespace BundleTests;
	CERegisterModuleTypes();

	TestBundleResolver resolver{};

	Bundle::PushBundleResolver(&resolver);

	// 1. Write
	{
		constexpr int NumTextures = 2;
		Ref<Bundle> transient = GetGlobalTransient();

		Ref<Bundle> scriptBundle = CreateObject<Bundle>(transient.Get(), "MyScriptBundle");
		Ref<Bundle> meshBundle = CreateObject<Bundle>(transient.Get(), "MyMeshBundle");
		Ref<Bundle> materialBundle = CreateObject<Bundle>(transient.Get(), "MyMaterialBundle");

		Array<Ref<Bundle>> textureBundles;

		Ref<MyMaterial> material = CreateObject<MyMaterial>(materialBundle.Get(), "MyMaterial");

		material->properties.Resize(3);
		material->properties[0].name = "Albedo";
		material->properties[1].name = "Normal";
		material->properties[2].name = "Roughness";

		{
			Ref<MyTexture> fallback = CreateObject<MyTexture>(materialBundle.Get(), "FallbackTexture");
			fallback->desc.width = fallback->desc.height = 16;

			material->fallbackTexture = fallback;
		}

		for (int i = 0; i < NumTextures; ++i)
		{
			Ref<Bundle> textureBundle = CreateObject<Bundle>(transient.Get(), String::Format("MyTextureBundle_{}", i));
			textureBundles.Add(textureBundle);

			resolver.bundlePathsByUuid[textureBundle->GetUuid()] = "/" + textureBundle->GetName().GetString();

			Ref<MyTexture> textureAsset = CreateObject<MyTexture>(textureBundle.Get(), "MyTexture");

			textureAsset->desc.width = (i + 1) * 512;
			textureAsset->desc.height = (i + 1) * 512;
			textureAsset->desc.filterMode = FilterMode::Trilinear;

			material->textures.Add(textureAsset);
		}

		Ref<MyMesh> meshAsset = CreateObject<MyMesh>(meshBundle.Get(), "MyMesh");
		meshAsset->callCounter = 0;
		meshAsset->material = material;

		Ref<MyScript> myScript = CreateObject<MyScript>(scriptBundle.Get(), "MyScript");
		myScript->scriptEvent.Bind(FUNCTION_BINDING(meshAsset.Get(), UpdateMesh));
		myScript->meshAsset = meshAsset;

		resolver.bundlePathsByUuid[scriptBundle->GetUuid()] = "/" + scriptBundle->GetName().GetString();
		resolver.bundlePathsByUuid[meshBundle->GetUuid()] = "/" + meshBundle->GetName().GetString();
		resolver.bundlePathsByUuid[materialBundle->GetUuid()] = "/" + materialBundle->GetName().GetString();

		Bundle::SaveToDisk(scriptBundle);
		Bundle::SaveToDisk(meshBundle);
		Bundle::SaveToDisk(materialBundle);

		for (const auto& textureBundle : textureBundles)
		{
			Bundle::SaveToDisk(textureBundle);
		}

		scriptBundle->BeginDestroy();
		meshBundle->BeginDestroy();
		materialBundle->BeginDestroy();
		for (const auto& textureBundle : textureBundles)
		{
			textureBundle->BeginDestroy();
		}
	}

	// 2. Read
	{
		LoadBundleArgs args{
			.loadFully = true,
			.forceReload = false,
			.destroyOutdatedObjects = true
		};

		Ref<Bundle> transient = CreateObject<Bundle>(GetGlobalTransient(), "LocalTransient", OF_Transient);

		// MyScript object stores reference to objects from other bundles. Hence all other bundles will be loaded automatically.
		Ref<Bundle> scriptBundle = Bundle::LoadBundle(transient, "/MyScriptBundle");

		EXPECT_EQ(scriptBundle->GetSubObjectCount(), 1);

		Ref<MyScript> myScript = (Ref<MyScript>)scriptBundle->LoadObject("MyScript");
		Ref<MyMesh> myMesh = myScript->meshAsset;

		EXPECT_TRUE(myScript->meshAsset.IsValid());
		EXPECT_EQ(myScript->scriptEvent.GetInvocationListCount(), 1);
		Array<FunctionBinding> bindings = myScript->scriptEvent.GetSerializableBindings();
		EXPECT_EQ(bindings.GetSize(), 1);
		EXPECT_EQ(bindings[0].object, myMesh);

		myScript->scriptEvent(myScript.Get());
		EXPECT_EQ(myMesh->callCounter, 1);
		myScript->scriptEvent(myScript.Get());
		EXPECT_EQ(myMesh->callCounter, 2);

		myMesh->callCounter = 0;

		Ref<MyMaterial> material = myMesh->material;
		EXPECT_TRUE(material.IsValid());

		EXPECT_EQ(material->properties.GetSize(), 3);
		EXPECT_EQ(material->properties[0].name, "Albedo");
		EXPECT_EQ(material->properties[1].name, "Normal");
		EXPECT_EQ(material->properties[2].name, "Roughness");

		EXPECT_EQ(material->textures.GetSize(), 2);
		EXPECT_EQ(material->textures[0]->desc.width, 512);
		EXPECT_EQ(material->textures[0]->desc.height, 512);
		EXPECT_EQ(material->textures[0]->desc.filterMode, FilterMode::Trilinear);
		EXPECT_EQ(material->textures[1]->desc.width, 1024);
		EXPECT_EQ(material->textures[1]->desc.height, 1024);
		EXPECT_EQ(material->textures[1]->desc.filterMode, FilterMode::Trilinear);
		EXPECT_EQ(material->fallbackTexture->desc.width, 16);
		EXPECT_EQ(material->fallbackTexture->desc.height, 16);

		transient->BeginDestroy();
	}

	Bundle::PopBundleResolver(&resolver);

	{
		IO::Path launchPath = PlatformDirectories::GetLaunchDir();
		Array<IO::Path> pathsToRemove;

		launchPath.IterateChildren([&](const IO::Path& path)
			{
				if (!path.IsDirectory() && path.GetExtension() == ".casset")
				{
					pathsToRemove.Add(path);
				}
			});

		for (const auto& path : pathsToRemove)
		{
			IO::Path::Remove(path);
		}
	}

	CEDeregisterModuleTypes();
	TEST_END;
}

#pragma endregion


/**********************************************
* Resources
*/

#pragma region Resource

static const char Resource_Text_Entry0[] = "resource_text";
static const char Resource_CSS_Style[] = R"(
* {
	padding: 5px 5px;
}
)";

TEST(Resource, Manipulation)
{
	TEST_BEGIN;

	GetResourceManager()->RegisterResource("Core_Test", "Text/Entry0.txt", (void*)Resource_Text_Entry0, COUNTOF(Resource_Text_Entry0));
	GetResourceManager()->RegisterResource("Core_Test", "CSS/Style.css", (void*)Resource_CSS_Style, COUNTOF(Resource_CSS_Style));

	Resource* resource = GetResourceManager()->LoadResource("/Core_Test/Resources/Text/Entry0.txt");
	EXPECT_EQ(strcmp((const char*)resource->GetData(), Resource_Text_Entry0), 0);

	Resource* cssResource = GetResourceManager()->LoadResource("/Core_Test/Resources/CSS/Style.css");
	EXPECT_EQ(strcmp((const char*)cssResource->GetData(), Resource_CSS_Style), 0);

	String textResource = GetResourceManager()->LoadTextResource("/Core_Test/Resources/Text/Entry0.txt");
	EXPECT_EQ(textResource, "resource_text");

	cssResource->BeginDestroy();
	cssResource = nullptr;
	resource->BeginDestroy();
	resource = nullptr;

	GetResourceManager()->DeregisterResource("Core_Test", "Text/Entry0.txt");
	GetResourceManager()->DeregisterResource("Core_Test", "CSS/Style.css");

	TEST_END;
}

#pragma endregion


/**********************************************
*  Job System
*/

#pragma region Job System

std::mutex globalMut{};

class JobSleep : public Job
{
public:

	JobSleep(SIZE_T sleepFor) : millis(sleepFor)
	{

	}

	void Process() override
	{
		//globalMut.lock();
		//LOG("JobSleep: " << millis);
		//globalMut.unlock();

		Thread::SleepFor(millis);
	}

	SIZE_T millis = 0;
};

TEST(JobSystem, Basic)
{
	TEST_BEGIN;

	{
		JobManagerDesc desc{};
		desc.totalThreads = 0;

		JobManager manager{ "Test", desc };
		JobContext context{ &manager };
		JobContext::PushGlobalContext(&context);

		auto prev = clock();
		int numThreads = 0;
		numThreads = manager.GetNumThreads();
		numThreads = Math::Max(2, numThreads); // Run only 2 threads
		
		for (int i = 0; i < numThreads; i++)
		{
			JobSleep* job = new JobSleep(500);
			job->Start();
		}

		manager.DeactivateWorkersAndWait();

		auto now = clock();
		f32 deltaTime = ((f32)(now - prev)) / CLOCKS_PER_SEC;
		EXPECT_LE(deltaTime, 0.8);

		JobContext::PopGlobalContext();
	}

	TEST_END;
}

TEST(JobSystem, Jobs)
{
	TEST_BEGIN;

	{
		JobManagerDesc desc{};
		desc.totalThreads = 0;

		JobManager manager{ "Test", desc };
		JobContext context{ &manager };
		JobContext::PushGlobalContext(&context);

		auto prev = clock();
		int numThreads = 0;
		numThreads = manager.GetNumThreads();
		numThreads = Math::Max(2, numThreads); // Run only 2 threads

		for (int i = 0; i < numThreads; i++)
		{
			Job* job = new JobFunction([&](Job* job)
				{
					Thread::SleepFor(500); // 500 milliseconds
				});
			job->Start();
		}

		manager.DeactivateWorkersAndWait();

		auto now = clock();
		f32 deltaTime = ((f32)(now - prev)) / CLOCKS_PER_SEC;
		EXPECT_LE(deltaTime, 0.8);

		JobContext::PopGlobalContext();
	}

	TEST_END;
}

class JobDep : public Job
{
public:

	JobDep(SIZE_T sleepFor) : millis(sleepFor)
	{

	}

	void Process() override
	{
		Thread::SleepFor(millis);
	}

	void Finish() override
	{

	}

	SIZE_T millis = 0;
};

class ReusableJob : public Job
{
public:

	ReusableJob(SIZE_T sleepFor) : Job(false), millis(sleepFor)
	{

	}

	void Process() override
	{
		Thread::SleepFor(millis);
	}

	void Finish() override
	{

	}

	SIZE_T millis = 0;
};

TEST(JobSystem, Dependency)
{
	TEST_BEGIN;

	// 1. Multiple dependencies
	{
		auto prev = clock();

		JobManagerDesc desc{};
		desc.totalThreads = 4;

		JobManager manager{ "Test", desc };
		JobContext context{ &manager };
		JobContext::PushGlobalContext(&context);

		JobDep* job0 = new JobDep(250);
		JobDep* job1 = new JobDep(100);
		JobDep* job2 = new JobDep(250);
		job0->SetDependent(job1);
		job2->SetDependent(job1);
		job2->Start();
		job1->Start();
		job0->Start();

		// Wait for jobs to complete & worker threads to deactivate
		manager.Complete();

		auto now = clock();
		f32 deltaTime = ((f32)(now - prev)) / CLOCKS_PER_SEC;
		EXPECT_LE(deltaTime, 0.45); // deltaTime = (250 OR 250) + 100 = 350 (we use 500 to account for other delays)

		JobContext::PopGlobalContext();
	}

	// 2. Job sequence
	{
		auto prev = clock();

		JobManagerDesc desc{};
		desc.totalThreads = 4;

		JobManager manager{ "Test", desc };
		JobContext context{ &manager };
		JobContext::PushGlobalContext(&context);

		ReusableJob* job0 = new ReusableJob(250);
		ReusableJob* job1 = new ReusableJob(100);
		job1->Start();
		job0->Start();
		job0->Complete();
		job1->Complete();
		// 250ms OR 100ms => 250ms

		job1->Reset(true);
		job1->Start();
		// 100ms

		job1->Complete();

		// Total: 350ms
		auto now = clock();
		f32 deltaTime = ((f32)(now - prev)) / CLOCKS_PER_SEC;
		EXPECT_LE(deltaTime, 0.5); // Should be 350ms (we use 500 to account for other delays)

		// Wait for worker threads to complete & deactivate
		manager.Complete();

		delete job0;
		delete job1;

		JobContext::PopGlobalContext();
	}

	TEST_END;
}

class FilteredJob : public Job
{
public:

	FilteredJob(SIZE_T sleepFor, int& i) : Job(false), millis(sleepFor), i(i)
	{

	}

	void Process() override
	{
		globalMut.lock();
		i = GetContext()->GetJobManager()->GetCurrentJobThreadIndex();
		globalMut.unlock();

		Thread::SleepFor(millis);
	}

	SIZE_T millis;
	int& i;
};

TEST(JobSystem, ThreadFilter)
{
	TEST_BEGIN;

	{
		JobManagerDesc desc{};
		desc.totalThreads = 4;
		desc.threads = {
			{ "Primary", JOB_THREAD_PRIMARY },
			{ "Asset", JOB_THREAD_ASSET },
		};

		JobManager manager{ "Test", desc };
		JobContext context{ &manager };
		JobContext::PushGlobalContext(&context);

		// thread 0 idle

		int i0 = -1, i1 = -1, i2 = -1, i3 = -1;

		FilteredJob* job0 = new FilteredJob(100, i0);
		job0->SetThreadFilter(JOB_THREAD_ASSET); // thread 1

		FilteredJob* job1 = new FilteredJob(50, i1);
		job1->SetThreadFilter(JOB_THREAD_WORKER); // thread 2 or 3

		FilteredJob* job2 = new FilteredJob(50, i2);
		job2->SetThreadFilter(JOB_THREAD_WORKER); // thread 2 or 3

		FilteredJob* job3 = new FilteredJob(50, i3);
		job3->SetThreadFilter(JOB_THREAD_WORKER); // thread 2 or 3

		job0->Start();
		job1->Start();
		job2->Start();
		job3->Start();

		manager.Complete();

		EXPECT_TRUE(i0 == 1);
		EXPECT_TRUE(i1 == 2 || i1 == 3);
		EXPECT_TRUE(i2 == 2 || i2 == 3);
		EXPECT_TRUE(i3 == 2 || i3 == 3);

		delete job0;
		delete job1;
		delete job2;
		delete job3;

		JobContext::PopGlobalContext();
	}

	TEST_END;
}

class ChildJob : public Job
{
public:

	ChildJob(int& i, int value) : i(i), value(value)
	{

	}

	String GetName() const override { return "Child Job"; }

	void Process() override
	{
		int threadIdx = GetContext()->GetJobManager()->GetCurrentJobThreadIndex();

		//globalMut.lock();
		//LOG("Child: " << value << " | " << threadIdx);
		//globalMut.unlock();

		Thread::SleepFor(10);
		i = value;
	}

	int& i;
	int value = 0;
};

class ParentJob : public Job
{
public:

	ParentJob(int& i) : i(i)
	{

	}

	String GetName() const override { return "Parent Job"; }

	void Process() override
	{
		int threadIdx = GetContext()->GetJobManager()->GetCurrentJobThreadIndex();

		ChildJob* child0 = new ChildJob(i, 0);
		ChildJob* child1 = new ChildJob(i, 1);
		ChildJob* child2 = new ChildJob(i, 2);
		ChildJob* child3 = new ChildJob(i, 3);
		ChildJob* child4 = new ChildJob(i, 4);

		StartAsChild(child0);
		StartAsChild(child1);
		StartAsChild(child2);
		StartAsChild(child3);
		StartAsChild(child4);

		i = -2;

		WaitForChildren();

		//globalMut.lock();
		//LOG("Parent | " << threadIdx);
		//globalMut.unlock();
	}

	int& i;
};

TEST(JobSystem, ChildJobs)
{
	TEST_BEGIN;

	{
		JobManagerDesc desc{};
		desc.totalThreads = 4;
		
		JobManager manager{ "Test", desc };
		JobContext context{ &manager };
		JobContext::PushGlobalContext(&context);

		int i = -1;

		ParentJob* parent = new ParentJob(i);
		parent->Start();
		
		manager.Complete();
		EXPECT_TRUE(i >= 0); // Child jobs always set i to >= 0

		JobContext::PopGlobalContext();
	}

	TEST_END;
}

TEST(JobSystem, Performance)
{
	TEST_BEGIN;

	{
		JobManagerDesc desc{};
		desc.totalThreads = 0;

		JobManager manager{ "Test", desc };
		JobContext context{ &manager };
		JobContext::PushGlobalContext(&context);

		auto prev = clock();
		int numThreads = 0;
		numThreads = manager.GetNumThreads();

		//for (int i = 0; i < 1; i++)
		{
			JobSleep* job = new JobSleep(100);
			job->Start();
			job->Complete();
		}

		{
			JobSleep* job0 = new JobSleep(100);
			JobSleep* job1 = new JobSleep(100);
			JobSleep* job2 = new JobSleep(100);
			JobSleep* job3 = new JobSleep(100);
			
			job0->Start();
			job1->Start();
			job2->Start();
			job3->Start();
		}

		manager.Complete();

		auto now = clock();
		f32 deltaTime = ((f32)(now - prev)) / CLOCKS_PER_SEC;
		EXPECT_LE(deltaTime, 0.3f); // <= 200ms

		JobContext::PopGlobalContext();
	}

	TEST_END;
}

#pragma endregion

