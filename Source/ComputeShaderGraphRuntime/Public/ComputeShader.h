#pragma once

#include "CoreMinimal.h"

#include "ComputeShader.generated.h"

class UComputeShaderNode;

#if WITH_EDITOR

class UEdGraph;
class UComputeShader;
class UComputeShaderNode;

/** Interface for interaction with the ComputeShaderGraph editor module. */
class IComputeShaderEdGraphEditor
{
public:
    virtual ~IComputeShaderEdGraphEditor() {}

    /** Called when creating a new compute shader graph. */
    virtual UEdGraph* CreateNewComputeShaderGraph(UComputeShader* InComputeShader) = 0;

    /** Sets up a compute shader node. */
    virtual void SetupComputeShaderNode(UEdGraph* ComputeShaderGraph, UComputeShaderNode* InComputeShaderNode, bool bSelectNewNode) = 0;

    /** Links graph nodes from compute shader nodes. */
    virtual void LinkGraphNodesFromComputeShaderNodes(UComputeShader* InComputeShader) = 0;

    /** Compiles compute shader nodes from graph nodes. */
    virtual void CompileFromGraphNodes(UComputeShader* InComputeShader) = 0;

    /** Removes nodes which are null from the compute shader graph. */
    virtual void RemoveNullNodes(UComputeShader* InComputeShader) = 0;

    /** Creates an input pin on the given compute shader graph node. */
    virtual void CreateInputPin(UEdGraphNode* ComputeShaderGraphNode) = 0;

    /** Renames all pins in a compute shader node */
    virtual void RenameNodePins(UComputeShaderNode* InComputeShaderNode) = 0;
};
#endif // WITH_EDITOR

UCLASS(BlueprintType)
class COMPUTESHADERGRAPHRUNTIME_API UComputeShader : public UObject
{
	GENERATED_BODY()
	
public:
    UComputeShader(const FObjectInitializer& Initializer);
    
    /** Construct and initialize a node within this ComputeShader */
    template<class T>
    T* ConstructComputeShaderNode(TSubclassOf<UComputeShaderNode> ComputeShaderNodeClass = T::StaticClass(), bool bSelectNewNode = true)
    {
        // Set flag to be transactional so it registers with undo system
        T* ComputeShaderNode = NewObject<T>(this, ComputeShaderNodeClass, NAME_None, RF_Transactional);
#if WITH_EDITOR
        AllNodes.Add(ComputeShaderNode);
        SetupComputeShaderNode(ComputeShaderNode, bSelectNewNode);
#endif // WITH_EDITORONLY_DATA
        return ComputeShaderNode;
    }

#if WITH_EDITOR
    virtual void PostInitProperties() override;

    /** Use the EdGraph representation to compile the SoundCue */
    void CompileFromGraphNodes();

    /** Set up EdGraph parts of a ComputeShader */
    void SetupComputeShaderNode(UComputeShaderNode* InComputeShader, bool bSelectNewNode = true);

    /** Use the ComputeShader's children to link EdGraph Nodes together */
    void LinkGraphNodesFromComputeShaderNodes();

    /** Get the EdGraph of SoundNodes */
    UEdGraph* GetGraph();

    /** Create the basic sound graph */
    void CreateGraph();

    /** Clears all nodes from the graph (for old editor's buffer compute shader) */
    void ClearGraph();

    /** Resets all graph data and nodes */
    void ResetGraph();

    /** Sets graph editor implementation.* */
    static void SetComputeShaderEdGraphEditor(TSharedPtr<IComputeShaderEdGraphEditor> InEditor);

    /** Gets graph editor implementation. */
    static TSharedPtr<IComputeShaderEdGraphEditor> GetComputeShaderEdGraphEditor();
#endif

public:
    UPROPERTY()
    UComputeShaderNode* FirstNode;

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    TArray<UComputeShaderNode*> AllNodes;

    UPROPERTY()
    UEdGraph* ComputeShaderGraph;
#endif

};