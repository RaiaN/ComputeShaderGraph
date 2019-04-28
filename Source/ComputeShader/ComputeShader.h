// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"


template< typename T >
class FClearTexture2DReplacementCS : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FClearTexture2DReplacementCS, Global, COMPUTESHADER_API);
public:
	FClearTexture2DReplacementCS() {}
	FClearTexture2DReplacementCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
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
	
	COMPUTESHADER_API void SetParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIParamRef TextureRW, const T(&Values)[4]);
	COMPUTESHADER_API void FinalizeParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIParamRef TextureRW);
	
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine( TEXT("Type"), TIsSame< T, float >::Value ? TEXT("float4") : TEXT("uint4") );
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