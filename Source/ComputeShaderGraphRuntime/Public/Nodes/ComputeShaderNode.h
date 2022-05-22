#pragma once

#include "CoreMinimal.h"
#include "ComputeShaderNode.generated.h"

class UEdGraphNode;

UCLASS(abstract, hidecategories = Object, editinlinenew)
class COMPUTESHADERGRAPHRUNTIME_API UComputeShaderNode : public UObject
{
    GENERATED_BODY()

public:
	/**
	* Returns an array of all (not just active) nodes.
	*/
	virtual void GetAllNodes(TArray<UComputeShaderNode*>& OutComputeShaderNodes);

	/**
	* Returns the maximum number of child nodes this node can possibly have
	*/
	virtual int32 GetMaxChildNodes() const
	{
		return 1;
	}

	/** Returns the minimum number of child nodes this node must have */
	virtual int32 GetMinChildNodes() const
	{
		return 0;
	}

    virtual void CreateStartingConnectors(void);
    virtual void InsertChildNode(int32 Index);
    virtual void RemoveChildNode(int32 Index);

#if WITH_EDITORONLY_DATA
    /** Node's Graph representation, used to get position. */
    UPROPERTY()
    UEdGraphNode* GraphNode;

	UEdGraphNode* GetGraphNode() const
	{
		return GraphNode;
	}
#endif

#if WITH_EDITOR
	/**
    * Set the entire Child Node array directly, allows GraphNodes to fully control node layout.
    * Can be overwritten to set up additional parameters that are tied to children.
    */
	virtual void SetChildNodes(TArray<UComputeShaderNode*>& InChildNodes);

    virtual FText GetInputPinName(int32 PinIndex) const { return FText::GetEmpty(); }
    virtual FText GetTitle() const { return GetClass()->GetDisplayNameText(); }
    /** Helper function to set the position of a sound node on a grid */
	// TODO
	// FIXME
    void PlaceNode(int32 NodeColumn, int32 NodeRow, int32 RowCount) { }
#endif

public:
    UPROPERTY()
    TArray<UComputeShaderNode*> ChildNodes;
};