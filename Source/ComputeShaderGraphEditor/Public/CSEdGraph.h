// Copyright Peter Leontev

#include "CoreMinimal.h"

#include "CSEdGraph.generated.h"


class UComputeShader;


UCLASS(MinimalAPI) 
class UComputeShaderEdGraph : public UEdGraph
{
    GENERATED_BODY()

public:
    /** Returns the ComputeShader that contains this graph */
    UComputeShader* GetComputeShader() const;
};