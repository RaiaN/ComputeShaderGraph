// Copyright Epic Games, Inc. All Rights Reserved.

#include "SComputeShaderPalette.h"
#include "EdGraph/CSGraphSchema.h"

void SComputeShaderPalette::Construct(const FArguments& InArgs)
{
	// Auto expand the palette as there's so few nodes
	SGraphPalette::Construct(SGraphPalette::FArguments().AutoExpandActionMenu(true));
}

void SComputeShaderPalette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	const UComputeShaderGraphSchema* Schema = GetDefault<UComputeShaderGraphSchema>();

	FGraphActionMenuBuilder ActionMenuBuilder;

	// Determine all possible actions
	Schema->GetPaletteActions(ActionMenuBuilder);

	//@TODO: Avoid this copy
	OutAllActions.Append(ActionMenuBuilder);
}
