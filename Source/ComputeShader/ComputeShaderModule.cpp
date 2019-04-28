#include "ComputeShaderModule.h" 
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

void IComputeShaderModule::StartupModule()
{
    // Maps virtual shader source directory /Plugin/ComputeShader to the plugin's actual Shaders directory.
    FString PluginShadersDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ComputeShader"))->GetBaseDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Plugin/ComputeShader"), PluginShadersDir);
}

void IComputeShaderModule::ShutdownModule()
{
	
}

IMPLEMENT_MODULE(IComputeShaderModule, ComputeShader)