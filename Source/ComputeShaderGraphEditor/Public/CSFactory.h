// Copyright Epic Games, Inc. All Rights Reserved.

//~=============================================================================
// ComputeShaderFactory
//~=============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "CSFactory.generated.h"

class UComputeShader;

UCLASS(hidecategories=Object, MinimalAPI)
class UComputeShaderFactory : public UFactory
{
	GENERATED_BODY()

public:
	UComputeShaderFactory(const FObjectInitializer& ObjectInitializer);

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
	//~ Begin UFactory Interface	
};



