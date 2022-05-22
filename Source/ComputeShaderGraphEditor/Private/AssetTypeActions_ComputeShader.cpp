// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetTypeActions_ComputeShader.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"
#include "AssetTypeCategories.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ComputeShader.h"
#include "CSEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FAssetTypeActions_ComputeShader::GetSupportedClass() const
{
	return UComputeShader::StaticClass();
}

void FAssetTypeActions_ComputeShader::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
    auto ComputeShaders = GetTypedWeakObjectPtrs<UComputeShader>(InObjects);
    FAssetTypeActions_Base::GetActions(InObjects, Section);
}

void FAssetTypeActions_ComputeShader::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UComputeShader* ComputeShader = Cast<UComputeShader>(*ObjIt);
		if (IsValid(ComputeShader))
		{
            TSharedRef<FComputeShaderEditor> NewComputeShaderEditor(new FComputeShaderEditor());
			NewComputeShaderEditor->InitComputeShaderEditor(Mode, EditWithinLevelEditor, ComputeShader);
		}
	}
}

uint32 FAssetTypeActions_ComputeShader::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
