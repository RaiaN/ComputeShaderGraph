// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "CSGraphNode_Base.h"
#include "CSGraphNode_Root.generated.h"

UCLASS(MinimalAPI)
class UComputeShaderGraphNode_Root : public UComputeShaderGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode interface
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	// End of UEdGraphNode interface

	// UComputeShaderGraphNode_Base
	virtual void CreateInputPins() override;
	virtual bool IsRootNode() const override {return true;}
	// ~UComputeShaderGraphNode_Base
};
