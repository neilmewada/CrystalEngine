
#include "Engine.h"

#include <iostream>
#include <any>

#include <gtest/gtest.h>

#include <iostream>

using namespace CE;

#define TEST_BEGIN BeginTest();
#define TEST_END EndTest();

static inline void BeginTest()
{
	CE::ModuleManager::Get().LoadModule("Core");
	CE::ModuleManager::Get().LoadModule("CoreMedia");
	CE::ModuleManager::Get().LoadModule("Engine");
}

static inline void EndTest()
{
	CE::ModuleManager::Get().UnloadModule("Engine");
	CE::ModuleManager::Get().UnloadModule("CoreMedia");
	CE::ModuleManager::Get().UnloadModule("Core");
}


#pragma region Nodes

TEST(Scene, BasicNodes)
{
	TEST_BEGIN;

	Scene* scene = CreateObject<Scene>(nullptr, "TestScene");
	{
		auto root = scene->GetRoot();
		EXPECT_NE(root, nullptr);
		EXPECT_TRUE(root->GetName().IsValid());

		auto myNode = CreateObject<Node>(root, "MyNode");
		EXPECT_EQ(root->GetChildrenCount(), 1);
		EXPECT_EQ(root->GetChildAt(0), myNode);

		NodeComponent* myNodeComp = CreateObject<NodeComponent>(myNode, "MyNodeComponent");
		EXPECT_EQ(root->GetComponentCount(), 0);
		EXPECT_EQ(myNode->GetComponentCount(), 1);
		EXPECT_EQ(myNode->GetComponentAt(0), myNodeComp);

		auto duplicate = (Scene*)scene->Clone();
		auto root2 = duplicate->GetRoot();
		EXPECT_NE(root2, nullptr);
		EXPECT_NE(root, root2);
		EXPECT_TRUE(root2->GetName().IsValid());

		EXPECT_EQ(root2->GetChildrenCount(), 1);
		auto myNodeCopy = root2->GetChildAt(0);
		EXPECT_NE(myNodeCopy, myNode);
		EXPECT_EQ(myNodeCopy->GetName(), "MyNode");

		EXPECT_EQ(myNodeCopy->GetComponentCount(), 1);
		auto myNodeCompCopy = myNodeCopy->GetComponentAt(0);
		EXPECT_NE(myNodeCompCopy, myNodeComp);
		EXPECT_EQ(myNodeCompCopy->GetName(), "MyNodeComponent");

		duplicate->Destroy();
	}
	scene->Destroy();

	TEST_END;
}

#pragma endregion

