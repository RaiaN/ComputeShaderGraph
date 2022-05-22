#pragma once

#include "CoreMinimal.h"
#include "ComputeShaderNode.h"
#include "ComputeShaderNode_Noise.generated.h"


UCLASS(hidecategories = Object, editinlinenew, meta = (DisplayName = "Noise"))
class UComputeShaderNode_Noise : public UComputeShaderNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, meta=(UIMin=0, UIMax=255))
    int32 MinValue;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0, UIMax = 255))
    int32 MaxValue;
};