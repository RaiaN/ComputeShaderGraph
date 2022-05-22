#pragma once

#include "CoreMinimal.h"
#include "ComputeShaderNode.h"
#include "ComputeShaderNode_Blend.generated.h"

class UTexture2D;

UCLASS(hidecategories = Object, editinlinenew, meta = (DisplayName = "Blend"))
class UComputeShaderNode_Blend : public UComputeShaderNode
{
    GENERATED_BODY()

protected:
    virtual int32 GetMaxChildNodes() const override { return 2; }
    virtual int32 GetMinChildNodes() const override { return 2; }
};