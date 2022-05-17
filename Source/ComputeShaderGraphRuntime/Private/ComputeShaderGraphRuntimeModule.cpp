// Copyright Peter Leontev

#include "ComputeShaderGraphRuntimeModule.h" 
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

void FComputeShaderGraphRuntimeModule::StartupModule()
{
    // Maps virtual shader source directory /Plugin/ComputeShader to the plugin's actual Shaders directory.
    FString PluginShadersDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ComputeShaderGraph"))->GetBaseDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Plugin/ComputeShaderGraph"), PluginShadersDir);
}

void FComputeShaderGraphRuntimeModule::ShutdownModule()
{
	
}

IMPLEMENT_MODULE(FComputeShaderGraphRuntimeModule, ComputeShaderGraphRuntime)