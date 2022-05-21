// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "CSGraphNode_Base.h"
#include "CSGraphNode.generated.h"

class UEdGraphPin;
class UComputeShaderNode;

UCLASS(MinimalAPI)
class UComputeShaderGraphNode : public UComputeShaderGraphNode_Base
{
	GENERATED_BODY()

public:
	/** The ComputeShaderNode this represents */
	UPROPERTY(VisibleAnywhere, instanced, Category=ComputeShader)
	UComputeShaderNode* ComputeShaderNode;

	/** Set the SoundNode this represents (also assigns this to the SoundNode in Editor)*/
	void SetComputeShaderNode(UComputeShaderNode* InComputeShaderNode);
	/** Fix up the node's owner after being copied */
	void PostCopyNode();
	/** Create a new input pin for this node */
	void CreateInputPin();
	/** Add an input pin to this node and recompile the ComputeShader */
	void AddInputPin();
	/** Remove a specific input pin from this node and recompile the ComputeShader */
	void RemoveInputPin(UEdGraphPin* InGraphPin);
	/** Estimate the width of this Node from the length of its title */
	int32 EstimateNodeWidth() const;
	/** Checks whether an input can be added to this node */
	bool CanAddInputPin() const;


	// UComputeShaderGraphNode_Base interface
	virtual void CreateInputPins() override;
	// End of UComputeShaderGraphNode_Base interface

	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PrepareForCopying() override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual FText GetTooltipText() const override;
	virtual FString GetDocumentationExcerptName() const override;
	// End of UEdGraphNode interface

	// UObject interface
	virtual void PostLoad() override;
	virtual void PostEditImport() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	// End of UObject interface

private:
	/** Make sure the soundnode is owned by the SoundCue */
	void ResetComputeShaderNodeOwner();
};
