// Copyright Peter Leontev

#include "ComputeShaderExample.h"
#include "ShaderParameterUtils.h"
#include "RendererInterface.h"
#include "RHIStaticStates.h"


void FTestFillTextureCS::SetParameters(FRHICommandList& RHICmdList, const FRWBufferStructured& TextureRW, const FColor& InFillColor, const uint32 InSize)
{
    SetShaderValue(RHICmdList, RHICmdList.GetBoundComputeShader(), FillColor, InFillColor.ToPackedRGBA());
    SetShaderValue(RHICmdList, RHICmdList.GetBoundComputeShader(), Size, InSize);
    SetUAVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), OutputBufferRW, TextureRW.UAV);
}

void FTestFillTextureCS::UnbindBuffers(FRHICommandList& RHICmdList)
{
    if (OutputBufferRW.IsBound())
    {
        RHICmdList.SetUAVParameter(RHICmdList.GetBoundComputeShader(), OutputBufferRW.GetBaseIndex(), FUnorderedAccessViewRHIRef());
    }
}

IMPLEMENT_SHADER_TYPE(, FTestFillTextureCS, TEXT("/Plugin/ComputeShaderGraph/Private/ExampleComputeShader.usf"), TEXT("FillBuffer"), SF_Compute);
