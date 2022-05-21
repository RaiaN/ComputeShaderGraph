#include "ComputeShaderNode.h"

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
    // TODO:
}

void UComputeShaderNode::InsertChildNode(int32 Index)
{
    // TODO:
}

void UComputeShaderNode::RemoveChildNode(int32 Index)
{
    // TODO:
}
