#pragma once

#include "CoreMinimal.h"
#include "ComputeShader.generated.h"

class UComputeShaderNode;

UCLASS(BlueprintType)
class COMPUTESHADERGRAPHRUNTIME_API UComputeShader : public UObject
{
	GENERATED_BODY()
	
public:
#if WITH_EDITOR
    /** Use the EdGraph representation to compile the SoundCue */
    void CompileSoundNodesFromGraphNodes();

    /** Set up EdGraph parts of a ComputeShader */
    void SetupComputeShaderNode(UComputeShaderNode* InComputeShader, bool bSelectNewNode = true);

    /** Get the EdGraph of SoundNodes */
    UEdGraph* GetGraph();
#endif

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

public:

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    TArray<UComputeShaderNode*> AllNodes;

    UPROPERTY()
    UEdGraph* SoundCueGraph;
#endif

};