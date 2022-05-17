// Copyright Peter Leontev

#pragma once

#include "Modules/ModuleManager.h"

class COMPUTESHADERGRAPHEDITOR_API FComputeShaderGraphEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

