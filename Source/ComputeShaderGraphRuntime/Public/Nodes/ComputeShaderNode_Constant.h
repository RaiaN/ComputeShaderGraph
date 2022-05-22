#pragma once

#include "CoreMinimal.h"
#include "ComputeShaderNode.h"
#include "ComputeShaderNode_Constant.generated.h"

class UTexture2D;

UCLASS(hidecategories = Object, editinlinenew, meta = (DisplayName = "Color"))
class UComputeShaderNode_Constant : public UComputeShaderNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    FLinearColor Color;
};