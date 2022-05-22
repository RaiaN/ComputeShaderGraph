// Copyright Peter Leontev

#include "CoreMinimal.h"

#include "CSEdGraph.generated.h"


class UComputeShader;


UCLASS() 
class COMPUTESHADERGRAPHEDITOR_API UComputeShaderEdGraph : public UEdGraph
{
    GENERATED_BODY()

public:
    UComputeShaderEdGraph(const FObjectInitializer& Initializer);

    /** Returns the ComputeShader that contains this graph */
    UComputeShader* GetComputeShader() const;
};