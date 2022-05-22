// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AssetData.h"
#include "EdGraph/EdGraphSchema.h"
#include "CSGraphSchema.generated.h"

class UEdGraph;
class UComputeShaderNode;

/** Action to add a node to the graph */
USTRUCT()
struct FComputeShaderGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_BODY();

	/** Class of node we want to create */
	UPROPERTY()
	class UClass* ComputeShaderNodeClass;

	FComputeShaderGraphSchemaAction_NewNode()
		: FEdGraphSchemaAction()
		, ComputeShaderNodeClass(NULL)
	{}

	FComputeShaderGraphSchemaAction_NewNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
		, ComputeShaderNodeClass(NULL)
	{}

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface

private:
	/** Connects new node to output of selected nodes */
	void ConnectToSelectedNodes(UComputeShaderNode* NewNodeclass, UEdGraph* ParentGraph) const;
};

///** Action to add nodes to the graph based on selected objects*/
//USTRUCT()
//struct COMPUTESHADERGRAPHEDITOR_API FComputeShaderGraphSchemaAction_NewFromSelected : public FComputeShaderGraphSchemaAction_NewNode
//{
//	GENERATED_USTRUCT_BODY();
//
//	FComputeShaderGraphSchemaAction_NewFromSelected()
//		: FSoundCueGraphSchemaAction_NewNode()
//	{}
//
//	FComputeShaderGraphSchemaAction_NewFromSelected(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
//		: FSoundCueGraphSchemaAction_NewNode(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping) 
//	{}
//
//	//~ Begin FEdGraphSchemaAction Interface
//	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
//	//~ End FEdGraphSchemaAction Interface
//};

///** Action to create new comment */
//USTRUCT()
//struct AUDIOEDITOR_API FSoundCueGraphSchemaAction_NewComment : public FEdGraphSchemaAction
//{
//	GENERATED_USTRUCT_BODY();
//
//	FSoundCueGraphSchemaAction_NewComment() 
//		: FEdGraphSchemaAction()
//	{}
//
//	FSoundCueGraphSchemaAction_NewComment(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
//		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
//	{}
//
//	//~ Begin FEdGraphSchemaAction Interface
//	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
//	//~ End FEdGraphSchemaAction Interface
//};
//
///** Action to paste clipboard contents into the graph */
//USTRUCT()
//struct AUDIOEDITOR_API FSoundCueGraphSchemaAction_Paste : public FEdGraphSchemaAction
//{
//	GENERATED_USTRUCT_BODY();
//
//	FSoundCueGraphSchemaAction_Paste() 
//		: FEdGraphSchemaAction()
//	{}
//
//	FSoundCueGraphSchemaAction_Paste(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
//		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
//	{}
//
//	//~ Begin FEdGraphSchemaAction Interface
//	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
//	//~ End FEdGraphSchemaAction Interface
//};

UCLASS(MinimalAPI)
class UComputeShaderGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	/** Check whether connecting these pins would cause a loop */
	bool ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const;

	/** Helper method to add items valid to the palette list */
	void GetPaletteActions(FGraphActionMenuBuilder& ActionMenuBuilder) const;

	/** Attempts to connect the output of multiple nodes to the inputs of a single one */
	void TryConnectNodes(const TArray<UComputeShaderNode*>& OutputNodes, UComputeShaderNode* InputNode) const;

	//~ Begin EdGraphSchema Interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
	virtual void GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const override;
	virtual void DroppedAssetsOnGraph(const TArray<struct FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const override;
	virtual void DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const override;
	virtual int32 GetNodeSelectionCount(const UEdGraph* Graph) const override;
	// FIXME
	// TODO
	// virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	//~ End EdGraphSchema Interface

private:
	/** Adds actions for creating every type of ComputeShaderNode */
	void GetAllComputeShaderNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, bool bShowSelectedActions) const;
	/** Adds action for creating a comment */
	void GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph = NULL) const;

private:
	/** Generates a list of all available ComputeShaderNode classes */
	static void InitComputeShaderNodeClasses();

	/** A list of all available ComputeShaderNode classes */
	static TArray<UClass*> ComputeShaderNodeClasses;
	/** Whether the list of the node classes has been populated */
	static bool bNodeClassesInitialized;
};

