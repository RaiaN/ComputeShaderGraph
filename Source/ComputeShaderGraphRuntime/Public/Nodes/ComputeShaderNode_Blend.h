#pragma once

#include "CoreMinimal.h"
#include "ComputeShaderNode.h"
#include "ComputeShaderNode_Blend.generated.h"

class UTexture2D;

UCLASS(hidecategories = Object, editinlinenew, meta = (DisplayName = "Blend"))
class UComputeShaderNode_Blend : public UComputeShaderNode
{
    GENERATED_BODY()

public:
    // TODO: blend mode
};