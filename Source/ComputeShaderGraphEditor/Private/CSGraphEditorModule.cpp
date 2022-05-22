// Copyright Peter Leontev

#include "CSGraphEditorModule.h" 
#include "AssetTypeActions_ComputeShader.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"

void FComputeShaderGraphEditorModule::StartupModule()
{
    // Register the compute shader editor asset type actions
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    AssetTools.RegisterAssetTypeActions(MakeShared<FAssetTypeActions_ComputeShader>());
}

void FComputeShaderGraphEditorModule::ShutdownModule()
{

}

IMPLEMENT_MODULE(FComputeShaderGraphEditorModule, ComputeShaderGraphEditor)