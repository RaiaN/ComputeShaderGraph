// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "ComputeShader.h"
#include "ShaderParameterUtils.h"
#include "RendererInterface.h"
#include "RHIStaticStates.h"


template< typename T >
void FTestFillTextureCS<T>::SetParameters(FRHICommandList& RHICmdList, const FRWBufferStructured& TextureRW, const FColor& InFillColor, const uint32 InSize)
{
    SetShaderValue(RHICmdList, RHICmdList.GetBoundComputeShader(), FillColor, InFillColor.ToPackedRGBA());
    SetShaderValue(RHICmdList, RHICmdList.GetBoundComputeShader(), Size, InSize);
    SetUAVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), OutputBufferRW, TextureRW.UAV);
}

template< typename T >
void FTestFillTextureCS<T>::UnbindBuffers(FRHICommandList& RHICmdList)
{
    if (OutputBufferRW.IsBound())
    {
        RHICmdList.SetUAVParameter(RHICmdList.GetBoundComputeShader(), OutputBufferRW.GetBaseIndex(), FUnorderedAccessViewRHIRef());
    }
}


template class FTestFillTextureCS<float>;
template class FTestFillTextureCS<uint32>;

IMPLEMENT_SHADER_TYPE(template<>, FTestFillTextureCS<float>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("FillBuffer"), SF_Compute);
IMPLEMENT_SHADER_TYPE(template<>, FTestFillTextureCS<uint32>, TEXT("/Plugin/ComputeShader/Private/ComputeShader.usf"), TEXT("FillBuffer"), SF_Compute);
