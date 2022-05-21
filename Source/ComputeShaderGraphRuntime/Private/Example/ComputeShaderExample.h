// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"



class FTestFillTextureCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FTestFillTextureCS, Global);
public:
    FTestFillTextureCS() {}
    FTestFillTextureCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader( Initializer )
	{
        FillColor.Bind(Initializer.ParameterMap, TEXT("FillColor"), SPF_Mandatory);
        Size.Bind(Initializer.ParameterMap, TEXT("Size"));
        OutputBufferRW.Bind(Initializer.ParameterMap, TEXT("OutputBufferRW"), SPF_Mandatory);
	}
	
	COMPUTESHADERGRAPHRUNTIME_API void SetParameters(FRHICommandList& RHICmdList, const FRWBufferStructured& TextureRW, const FColor& InFillColor, const uint32 InSize);

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
	LAYOUT_FIELD(FShaderParameter, FillColor);
	LAYOUT_FIELD(FShaderParameter, Size);
	LAYOUT_FIELD(FShaderResourceParameter, OutputBufferRW);
};