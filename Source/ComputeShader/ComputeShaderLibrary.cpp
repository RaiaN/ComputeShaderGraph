#include "ComputeShaderLibrary.h"
#include "ComputeShader.h"
#include "RHI.h"
#include "RHIResources.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderUtils.h"
#include "Shader.h"
#include "TextureResource.h"
#include "UObject/UnrealType.h"
#include "HAL/UnrealMemory.h"

PRAGMA_DISABLE_OPTIMIZATION



void ReadComputeShaderResult(FRHICommandListImmediate& RHICmdList, FRWBufferStructured& Buffer)
{
    const uint32 NumBytes = Buffer.NumBytes;
    const uint32 BytesToRead = Buffer.NumBytes / Buffer.Buffer->GetStride();

    TArray<uint32> BufferColorBytes;
    BufferColorBytes.SetNum(BytesToRead);

    uint32* BufferToRead = (uint32*)RHICmdList.LockStructuredBuffer(Buffer.Buffer, 0, NumBytes, RLM_ReadOnly);
    FMemory::Memcpy(BufferColorBytes.GetData(), BufferToRead, BytesToRead);
    RHIUnlockStructuredBuffer(Buffer.Buffer);

    UE_LOG(LogTemp, Warning, TEXT("Size: %d"), BufferColorBytes.Num());

   
    TArray<FColor> Bitmap;

    // if the format and texture type is supported
    if (Bitmap.Num())
    {
        // Create screenshot folder if not already present.
        IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);

        const FString ScreenFileName(FPaths::ScreenShotDir() / TEXT("VisualizeTexture"));

        // FFileHelper::CreateBitmap(*ScreenFileName, NumBytes / Buffer.Buffer->st, NumBytes / 4, Bitmap.GetData());

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

    const uint32 Size = 512;

    EPixelFormat BufferFormat = PF_A32B32G32R32F;
    uint32 BytesPerElement = GPixelFormats[BufferFormat].BlockBytes;

    FRWBufferStructured* RWBufferStructured = new FRWBufferStructured();
    RWBufferStructured->Initialize(BytesPerElement, Size);

    FRHIComputeShader* ShaderRHI = ComputeShader.GetComputeShader();
    RHICmdList.SetComputeShader(ShaderRHI);

    // FIXME: color not passed to compute shader
    ComputeShader->SetParameters(RHICmdList, *RWBufferStructured, FColor(255, 255, 128, 128), Size);
    RHICmdList.DispatchComputeShader(Size, 1, 1);
    ComputeShader->UnbindBuffers(RHICmdList);

    ReadComputeShaderResult(RHICmdList, *RWBufferStructured);
}

void UComputeShaderLibrary::UseEngineComputeShader(UTexture2D* InTexture)
{
    ENQUEUE_RENDER_COMMAND(FSetTextureColorCommand)(
    [](FRHICommandList& RHICmdList)
    {
        InnerUseComputeShader();
    });
}

PRAGMA_ENABLE_OPTIMIZATION