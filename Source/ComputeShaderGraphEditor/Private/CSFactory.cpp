// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSFactory.h"
#include "ComputeShader.h"

UComputeShaderFactory::UComputeShaderFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UComputeShader::StaticClass();

	bCreateNew = true;
	bEditorImport = false;
	bEditAfterNew = true;
}

UObject* UComputeShaderFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UComputeShader* ComputeShader = NewObject<UComputeShader>(InParent, Name, Flags);

	// Set default parameters
	
	return ComputeShader;
}
