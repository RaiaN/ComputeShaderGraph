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
        FillColor.Bind(Initializer.ParameterMap, TEXT("FillColor"), SPF_Mandatory);
        Size.Bind(Initializer.ParameterMap, TEXT("Size"));
        OutputBufferRW.Bind(Initializer.ParameterMap, TEXT("OutputBufferRW"), SPF_Mandatory);
	}
	
	COMPUTESHADER_API void SetParameters(FRHICommandList& RHICmdList, const FRWBufferStructured& TextureRW, const FColor& InFillColor, const uint32 InSize);

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
		return FillColor;
	}
	
	const FShaderResourceParameter& GetClearTextureRWParameter()
	{
		return OutputBufferRW;
	}
	
protected:
	FShaderParameter FillColor;
    FShaderParameter Size;
	FShaderResourceParameter OutputBufferRW;
};