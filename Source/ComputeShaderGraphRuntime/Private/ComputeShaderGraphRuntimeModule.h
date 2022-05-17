// Copyright Peter Leontev

#pragma once

#include "Modules/ModuleManager.h"

class COMPUTESHADERGRAPHRUNTIME_API FComputeShaderGraphRuntimeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

