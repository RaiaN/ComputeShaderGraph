#include "Nodes/ComputeShaderNode.h"
#include "ComputeShader.h"

void UComputeShaderNode::GetAllNodes(TArray<UComputeShaderNode*>& OutComputeShaderNodes)
{
    OutComputeShaderNodes.Add(this);
    for (int32 i = 0; i < ChildNodes.Num(); ++i)
    {
        if (ChildNodes[i])
        {
            ChildNodes[i]->GetAllNodes(OutComputeShaderNodes);
        }
    }
}

void UComputeShaderNode::CreateStartingConnectors(void)
{
    int32 ConnectorsToMake = FMath::Max(1, GetMinChildNodes());
    while (ConnectorsToMake > 0)
    {
        InsertChildNode(ChildNodes.Num());
        --ConnectorsToMake;
    }
}

void UComputeShaderNode::InsertChildNode(int32 Index)
{
    check( Index >= 0 && Index <= ChildNodes.Num() );
	int32 MaxChildNodes = GetMaxChildNodes();
	if (MaxChildNodes > ChildNodes.Num())
	{
		ChildNodes.InsertZeroed( Index );
#if WITH_EDITOR
        UComputeShader::GetComputeShaderEdGraphEditor()->CreateInputPin(GetGraphNode());
#endif //WITH_EDITORONLY_DATA
	}
}

void UComputeShaderNode::RemoveChildNode(int32 Index)
{
    check(Index >= 0 && Index < ChildNodes.Num());
    int32 MinChildNodes = GetMinChildNodes();
    if (ChildNodes.Num() > MinChildNodes)
    {
        ChildNodes.RemoveAt(Index);
    }
}

#if WITH_EDITOR
void UComputeShaderNode::SetChildNodes(TArray<UComputeShaderNode*>& InChildNodes)
{
    int32 MaxChildNodes = GetMaxChildNodes();
    int32 MinChildNodes = GetMinChildNodes();
    if (MaxChildNodes >= InChildNodes.Num() && InChildNodes.Num() >= MinChildNodes)
    {
        ChildNodes = InChildNodes;
    }
}
#endif //WITH_EDITOR
