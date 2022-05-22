// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UCSGraphNode_Root.cpp
=============================================================================*/

#include "CSGraphNode_Root.h"
#include "ToolMenus.h"
#include "GraphEditorSettings.h"
// #include "SoundCueGraphEditorCommands.h"

#define LOCTEXT_NAMESPACE "UComputeShaderGraphNode_Root"

/////////////////////////////////////////////////////
// USoundCueGraphNode_Root

UComputeShaderGraphNode_Root::UComputeShaderGraphNode_Root(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FLinearColor UComputeShaderGraphNode_Root::GetNodeTitleColor() const
{
	return GetDefault<UGraphEditorSettings>()->ResultNodeTitleColor;
}

FText UComputeShaderGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("RootTitle", "Output");
}

FText UComputeShaderGraphNode_Root::GetTooltipText() const
{
	return LOCTEXT("RootToolTip", "Wire the final Compute Shader Node into this node");
}

void UComputeShaderGraphNode_Root::CreateInputPins()
{
	CreatePin(EGPD_Input, TEXT("ComputeShaderNode"), TEXT("Root"), NAME_None);
}

void UComputeShaderGraphNode_Root::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin)
	{
	}
	else if (Context->Node)
	{
		// TODO
		// FIXME
        // FToolMenuSection& Section = Menu->AddSection("ComputeShaderGraphNodePlay");
        // Section.AddMenuEntry(FSoundCueGraphEditorCommands::Get().PlayNode);
	}
}

#undef LOCTEXT_NAMESPACE
