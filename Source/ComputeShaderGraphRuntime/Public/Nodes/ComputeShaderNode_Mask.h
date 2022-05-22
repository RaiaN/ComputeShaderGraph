#pragma once

#include "CoreMinimal.h"
#include "ComputeShaderNode.h"
#include "ComputeShaderNode_Mask.generated.h"


UCLASS(hidecategories = Object, editinlinenew, meta = (DisplayName = "Mask"))
class UComputeShaderNode_Mask : public UComputeShaderNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    bool bRed;

    UPROPERTY(EditAnywhere)
    bool bGreen;

    UPROPERTY(EditAnywhere)
    bool bBlue;

    UPROPERTY(EditAnywhere)
    bool bAlpha;
};