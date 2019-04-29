#include "ComputeShaderLibrary.h"
#include "ComputeShader.h"
#include "RHI.h"
#include "RHIResources.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "Shader.h"
#include "TextureResource.h"
#include "UObject/UnrealType.h"
#include "HAL/UnrealMemory.h"

PRAGMA_DISABLE_OPTIMIZATION


#define NUM_THREADS_PER_GROUP_DIMENSION 8 //This has to be the same as in the compute shader's spec [X, X, 1]


void ReadComputeShaderResult(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef Texture)
{
    TArray<FColor> Bitmap;

    //To access our resource we do a custom read using lockrect
    uint32 Stride = 0;
    char* TextureDataPtr = (char*)RHICmdList.LockTexture2D(Texture, 0, EResourceLockMode::RLM_ReadOnly, Stride, false);

    for (uint32 Row = 0; Row < Texture->GetSizeY(); ++Row)
    {
        uint32* RowDataPtr = (uint32*)TextureDataPtr;

        //Since we are using our custom UINT format, we need to unpack it here to access the actual colors
        for (uint32 Col = 0; Col < Texture->GetSizeX(); ++Col)
        {
            uint32 EncodedPixel = *RowDataPtr; 
            uint8 r = (EncodedPixel & 0x000000FF);
            uint8 g = (EncodedPixel & 0x0000FF00) >> 8;
            uint8 b = (EncodedPixel & 0x00FF0000) >> 16;
            uint8 a = (EncodedPixel & 0xFF000000) >> 24;

            Bitmap.Add(FColor(r, g, b, a));

            RowDataPtr++;
        }

        TextureDataPtr += Stride;
    }

    RHIUnlockTexture2D(Texture, 0, false);

    UE_LOG(LogTemp, Warning, TEXT("Size: %d"), Bitmap.Num());

    

    // if the format and texture type is supported
    if (Bitmap.Num())
    {
        // Create screenshot folder if not already present.
        IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);

        const FString ScreenFileName(FPaths::ScreenShotDir() / TEXT("VisualizeTexture"));

        // uint32 ExtendXWithMSAA = Bitmap.Num() / ;

        // Save the contents of the array to a bitmap file. (24bit only so alpha channel is dropped)
        FFileHelper::CreateBitmap(*ScreenFileName, Texture->GetSizeX(), Texture->GetSizeY(), Bitmap.GetData());

        UE_LOG(LogTemp, Display, TEXT("Content was saved to \"%s\""), *FPaths::ScreenShotDir());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
    }
}



void InnerUseComputeShader()
{
    FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

    TShaderMapRef<FTestFillTextureCS<float>> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

    FRHIResourceCreateInfo ResourceCreateInfo;

    FTexture2DRHIRef Texture2DRHIRef = RHICreateTexture2D(
        256, //InTexture->GetSizeX(),
        256, //InTexture->GetSizeY(),
        PF_R32_UINT,
        1,
        1,
        TexCreate_ShaderResource | TexCreate_UAV,
        ResourceCreateInfo
    );
    FUnorderedAccessViewRHIRef TextureUAV = RHICreateUnorderedAccessView(Texture2DRHIRef);

    
    // FIXME: color not passed to compute shader
    ComputeShader->SetParameters(RHICmdList, TextureUAV, FColor::Red);
 
    FComputeShaderRHIParamRef ShaderRHI = ComputeShader->GetComputeShader();
    RHICmdList.SetComputeShader(ShaderRHI);
   
    uint32 x = Texture2DRHIRef->GetSizeX() / NUM_THREADS_PER_GROUP_DIMENSION;
    uint32 y = Texture2DRHIRef->GetSizeY() / NUM_THREADS_PER_GROUP_DIMENSION;

    RHICmdList.DispatchComputeShader(x, y, 1);

    ComputeShader->UnbindBuffers(RHICmdList);

    ReadComputeShaderResult(RHICmdList, Texture2DRHIRef);

}

void UComputeShaderLibrary::UseEngineComputeShader(UTexture2D* InTexture)
{
    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
        FSetTextureColorCommand,
        UTexture2D*, Texture, InTexture,
        {
            InnerUseComputeShader();
        }
    );
}

PRAGMA_ENABLE_OPTIMIZATION