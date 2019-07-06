// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "ComputeShader.h"
#include "ShaderParameterUtils.h"
#include "RendererInterface.h"
#include "RHIStaticStates.h"


template< typename T >
void FTestFillTextureCS<T>::SetParameters(FRHICommandList& RHICmdList, const FRWBufferStructured& TextureRW, const FColor& InFillColor, const uint32 InSize)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();
    
    SetShaderValue(RHICmdList, ComputeShaderRHI, FillColor, InFillColor.ToPackedRGBA());
    SetShaderValue(RHICmdList, ComputeShaderRHI, Size, InSize);
    SetUAVParameter(RHICmdList, ComputeShaderRHI, OutputBufferRW, TextureRW.UAV);
}

template< typename T >
void FTestFillTextureCS<T>::UnbindBuffers(FRHICommandList& RHICmdList)
{
    FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

    if (OutputBufferRW.IsBound())
    {
        RHICmdList.SetUAVParameter(ComputeShaderRHI, OutputBufferRW.GetBaseIndex(), FUnorderedAccessViewRHIRef());
    }
}


template class FTestFillTextureCS<float>;
template class FTestFillTextureCS<uint32>;

IMPLEMENT_SHADER_TYPE(template<>, FTestFillTextureCS<float>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("FillBuffer"), SF_Compute);
IMPLEMENT_SHADER_TYPE(template<>, FTestFillTextureCS<uint32>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("FillBuffer"), SF_Compute);
