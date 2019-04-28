// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "ComputeShader.h"
#include "ShaderParameterUtils.h"
#include "RendererInterface.h"

template< typename T >
void FClearTexture2DReplacementCS<T>::SetParameters( FRHICommandList& RHICmdList, FUnorderedAccessViewRHIParamRef TextureRW, const T(&Values)[4] )
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();
	SetShaderValue(RHICmdList, ComputeShaderRHI, ClearColor, Values);

	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, TextureRW);
	RHICmdList.SetUAVParameter(ComputeShaderRHI, ClearTextureRW.GetBaseIndex(), TextureRW);
}

template< typename T >
void FClearTexture2DReplacementCS<T>::FinalizeParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIParamRef TextureRW)
{
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, TextureRW);
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();
	SetUAVParameter(RHICmdList, ComputeShaderRHI, ClearTextureRW, FUnorderedAccessViewRHIRef());
}

template class FClearTexture2DReplacementCS<float>;
template class FClearTexture2DReplacementCS<uint32>;

IMPLEMENT_SHADER_TYPE(template<>, FClearTexture2DReplacementCS<float>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("ClearTexture2DCS"), SF_Compute);
IMPLEMENT_SHADER_TYPE(template<>, FClearTexture2DReplacementCS<uint32>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("ClearTexture2DCS"), SF_Compute);
