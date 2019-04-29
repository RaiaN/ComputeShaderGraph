// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "ComputeShader.h"
#include "ShaderParameterUtils.h"
#include "RendererInterface.h"
#include "RHIStaticStates.h"


template< typename T >
void FTestFillTextureCS<T>::SetParameters( FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef TextureRW, const FColor& ClearColorValue)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();
    
    SetShaderValue(RHICmdList, ComputeShaderRHI, ClearColor, FVector4(ClearColorValue));
  

    if (ClearTextureRW.IsBound())
    {
        RHICmdList.SetUAVParameter(ComputeShaderRHI, ClearTextureRW.GetBaseIndex(), TextureRW);
    }
}

template< typename T >
void FTestFillTextureCS<T>::UnbindBuffers(FRHICommandList& RHICmdList)
{
    FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

    if (ClearTextureRW.IsBound())
    {
        RHICmdList.SetUAVParameter(ComputeShaderRHI, ClearTextureRW.GetBaseIndex(), FUnorderedAccessViewRHIRef());
    }
}


template class FTestFillTextureCS<float>;
template class FTestFillTextureCS<uint32>;

IMPLEMENT_SHADER_TYPE(template<>, FTestFillTextureCS<float>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("ClearTexture2DCS"), SF_Compute);
IMPLEMENT_SHADER_TYPE(template<>, FTestFillTextureCS<uint32>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("ClearTexture2DCS"), SF_Compute);
