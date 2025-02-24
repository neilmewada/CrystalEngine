#pragma once

CLASS()
class TestActorComponent : public ActorComponent
{
	CE_CLASS(TestActorComponent, ActorComponent)
public:



};

CLASS()
class TestFeatureProcessor1 : public RPI::FeatureProcessor
{
	CE_CLASS(TestFeatureProcessor1, RPI::FeatureProcessor)
public:



};

#include "TestFeatureProcessor.rtti.h"
