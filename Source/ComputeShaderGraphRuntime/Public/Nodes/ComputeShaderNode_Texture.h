#pragma once

#include "CoreMinimal.h"
#include "ComputeShaderNode.h"
#include "ComputeShaderNode_Texture.generated.h"

class UTexture2D;

UCLASS(hidecategories = Object, editinlinenew, meta = (DisplayName = "Texture"))
class UComputeShaderNode_Texture : public UComputeShaderNode
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
    UTexture2D* Texture;
};