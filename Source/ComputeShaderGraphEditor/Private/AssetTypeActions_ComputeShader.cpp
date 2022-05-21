// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetTypeActions_ComputeShader.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ComputeShader.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FAssetTypeActions_SoundCue::GetSupportedClass() const
{
	return UComputeShader::StaticClass();
}

void FAssetTypeActions_SoundCue::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
    auto SoundCues = GetTypedWeakObjectPtrs<UComputeShader>(InObjects);
    FAssetTypeActions_Base::GetActions(InObjects, Section);
}

void FAssetTypeActions_SoundCue::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto ComputeShader = Cast<UComputeShader>(*ObjIt);
		if (ComputeShader != NULL)
		{
			// FIXME
			// TODO
			// IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>( "AudioEditor" );
			// AudioEditorModule->CreateSoundCueEditor(Mode, EditWithinLevelEditor, ComputeShader);
		}
	}
}

#undef LOCTEXT_NAMESPACE
