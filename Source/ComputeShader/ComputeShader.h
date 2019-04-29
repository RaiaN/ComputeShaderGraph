// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"



template< typename T >
class FTestFillTextureCS : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FTestFillTextureCS, Global, COMPUTESHADER_API);
public:
    FTestFillTextureCS() {}
    FTestFillTextureCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader( Initializer )
	{
		ClearColor.Bind(Initializer.ParameterMap, TEXT("ClearColor"), SPF_Mandatory);
		ClearTextureRW.Bind(Initializer.ParameterMap, TEXT("ClearTextureRW"), SPF_Mandatory);
	}
	
	// FShader interface.
	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << ClearColor << ClearTextureRW;
		return bShaderHasOutdatedParameters;
	}
	
	COMPUTESHADER_API void SetParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef TextureRW, const FColor& ClearColorValue);

    void UnbindBuffers(FRHICommandList& RHICmdList);

	
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	
	const FShaderParameter& GetClearColorParameter()
	{
		return ClearColor;
	}
	
	const FShaderResourceParameter& GetClearTextureRWParameter()
	{
		return ClearTextureRW;
	}
	
protected:
	FShaderParameter ClearColor;
	FShaderResourceParameter ClearTextureRW;
};