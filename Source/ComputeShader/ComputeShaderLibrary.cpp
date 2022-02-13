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

#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "StaticMeshVertexData.h"
#include "Rendering/PositionVertexBuffer.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
#include "LocalVertexFactory.h"
#include "Components.h"


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

    TShaderMapRef<FTestFillTextureCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

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

PRAGMA_DISABLE_OPTIMIZATION

void UComputeShaderLibrary::SelectiveScale(UStaticMeshComponent* InComponent, UStaticMesh* InStaticMesh, const TArray<int32>& AffectedVertices, float ScaleFactor)
{
    FStaticMeshRenderData* RenderData = InStaticMesh->GetRenderData();
    
    FPrimitiveSceneProxy* SceneProxy = InComponent->SceneProxy;
    const FMeshBatch* MeshBatch = SceneProxy->GetPrimitiveSceneInfo()->GetMeshBatch(0);

    const FLocalVertexFactory* VertexFactory = static_cast<const FLocalVertexFactory*>(MeshBatch->VertexFactory);

    struct InitStaticMeshVertexFactoryParams
    {
        FLocalVertexFactory* VertexFactory;
        const FStaticMeshLODResources* LODResources;
        bool bOverrideColorVertexBuffer;
        uint32 LightMapCoordinateIndex;
        uint32 LODIndex;
    } Params;

    FLocalVertexFactory NewVertexFactory(GMaxRHIFeatureLevel, "");

    {
        FStaticMeshLODResources& LodResources = RenderData->LODResources[0];

        uint32 LightMapCoordinateIndex = (uint32)InStaticMesh->GetLightMapCoordinateIndex();
        LightMapCoordinateIndex = LightMapCoordinateIndex < LodResources.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords() ? LightMapCoordinateIndex : LodResources.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords() - 1;

        Params.VertexFactory = &NewVertexFactory;
        Params.LODResources = &LodResources;
        Params.bOverrideColorVertexBuffer = false;
        Params.LightMapCoordinateIndex = LightMapCoordinateIndex;
        Params.LODIndex = 0;
    }

    // BIND OUR VERTEX BUFFER
    // Initialize the static mesh's vertex factory.
    ENQUEUE_RENDER_COMMAND(InitStaticMeshVertexFactory)(
        [Params](FRHICommandListImmediate& RHICmdList)
        {
            FLocalVertexFactory::FDataType Data;

            Params.LODResources->VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(Params.VertexFactory, Data);
            Params.LODResources->VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(Params.VertexFactory, Data);
            Params.LODResources->VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(Params.VertexFactory, Data);
            Params.LODResources->VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(Params.VertexFactory, Data, Params.LightMapCoordinateIndex);

            // bOverrideColorVertexBuffer means we intend to override the color later.  We must construct the vertexfactory such that it believes a proper stride (not 0) is set for
            // the color stream so that the real stream works later.
            if (Params.bOverrideColorVertexBuffer)
            {
                FColorVertexBuffer::BindDefaultColorVertexBuffer(Params.VertexFactory, Data, FColorVertexBuffer::NullBindStride::FColorSizeForComponentOverride);
            }
            //otherwise just bind the incoming buffer directly.
            else
            {
                Params.LODResources->VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(Params.VertexFactory, Data);
            }

            Data.LODLightmapDataIndex = Params.LODIndex;
            Params.VertexFactory->SetData(Data);
            Params.VertexFactory->InitResource();
        });


    // SceneProxy->GetScene().UpdateCachedRenderStates(SceneProxy);
   
   
    
    for (FStaticMeshLODResources& LODResources : RenderData->LODResources)
    {
        FPositionVertexBuffer& PositionVertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;

        const int32 NumVertices = PositionVertexBuffer.GetNumVertices();
        
        FVertexBufferRHIRef PositionVertexBufferRHI = PositionVertexBuffer.VertexBufferRHI;

        FVertexBufferRHIRef TangentsVertexBuffer = LODResources.VertexBuffers.StaticMeshVertexBuffer.TangentsVertexBuffer.VertexBufferRHI;


        ENQUEUE_RENDER_COMMAND(GrabVertexBuffer)(
            [PositionVertexBufferRHI, NumVertices, AffectedVertices, ScaleFactor](FRHICommandListImmediate& RHICmdList)
            {
                PositionVertexBufferRHI->GetSize();

                void* VertexBufferData = RHILockVertexBuffer(PositionVertexBufferRHI, 0, PositionVertexBufferRHI->GetSize(), RLM_ReadOnly);

                // copy RHI vertex data to temporary buffer
                TArray<FPositionVertex> Positions;
                Positions.SetNum(NumVertices);

                FMemory::Memcpy(Positions.GetData(), VertexBufferData, PositionVertexBufferRHI->GetSize());
                    
                // now modify Positions
                for (int32 VertexIndex : AffectedVertices)
                {
                    if (VertexIndex >= 0 && VertexIndex < NumVertices)
                    {
                        Positions[VertexIndex].Position *= ScaleFactor;
                    }
                }

                RHIUnlockVertexBuffer(PositionVertexBufferRHI);

                // lock vertex buffer again to write modified positions
                VertexBufferData = RHILockVertexBuffer(PositionVertexBufferRHI, 0, PositionVertexBufferRHI->GetSize(), RLM_WriteOnly);

                FMemory::Memcpy(VertexBufferData, Positions.GetData(), PositionVertexBufferRHI->GetSize());

                RHIUnlockVertexBuffer(PositionVertexBufferRHI);
            }
        );
    }
}

PRAGMA_ENABLE_OPTIMIZATION