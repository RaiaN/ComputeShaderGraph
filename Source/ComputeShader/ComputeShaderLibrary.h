#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ComputeShaderLibrary.generated.h"


class UTexture2D;


/**
 * 
 */
UCLASS()
class COMPUTESHADER_API UComputeShaderLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static void UseEngineComputeShader(UTexture2D* InTexture);

    UFUNCTION(BlueprintCallable)
    static void SelectiveScale(UStaticMeshComponent* InComponent, UStaticMesh* InStaticMesh, const TArray<int32>& AffectedVertices, float ScaleFactor);
};
