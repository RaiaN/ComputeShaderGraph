// Copyright Peter Leontev

#include "CSEdGraph.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraphSchema.h"
#include "ComputeShader.h"
#include "ComputeShaderNode.h"
#include "ComputeShaderGraph/CSGraphSchema.h"
#include "ComputeShaderGraph/CSGraphNode_Root.h"
#include "ComputeShaderGraph/CSGraphNode_Base.h"
#include "ComputeShaderGraph/CSGraphNode.h"


class FComputeShaderEdGraphEditor : public IComputeShaderEdGraphEditor
{
    
public:
    UEdGraph* CreateNewComputeShaderGraph(UComputeShader* InComputeShader) override
    {
        UComputeShaderEdGraph* Graph = CastChecked<UComputeShaderEdGraph>(
            FBlueprintEditorUtils::CreateNewGraph(
                InComputeShader, NAME_None, UComputeShaderEdGraph::StaticClass(), UComputeShaderGraphSchema::StaticClass()
            )
        );
        return Graph;
    }

    void SetupComputeShaderNode(UEdGraph* ComputeShaderGraph, UComputeShaderNode* InComputeShaderNode, bool bSelectNewNode) override
    {
        FGraphNodeCreator<UComputeShaderGraphNode> NodeCreator(*ComputeShaderGraph);
        UComputeShaderGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
        GraphNode->SetComputeShaderNode(InComputeShaderNode);
        NodeCreator.Finalize();
    }


    void LinkGraphNodesFromComputeShaderNodes(UComputeShader* ComputeShader) override
    {
        // Use ComputeShaderNodes to make GraphNode Connections
        if (ComputeShader->FirstNode != NULL)
        {
            // Find the root node
            TArray<UComputeShaderGraphNode_Root*> RootNodeList;
            ComputeShader->ComputeShaderGraph->GetNodesOfClass<UComputeShaderGraphNode_Root>(/*out*/ RootNodeList);
            check(RootNodeList.Num() == 1);

            RootNodeList[0]->Pins[0]->BreakAllPinLinks();
            RootNodeList[0]->Pins[0]->MakeLinkTo(CastChecked<UComputeShaderGraphNode>(ComputeShader->FirstNode->GetGraphNode())->GetOutputPin());
        }

        for (TArray<UComputeShaderNode*>::TConstIterator It(ComputeShader->AllNodes); It; ++It)
        {
            UComputeShaderNode* Node = *It;
            if (Node)
            {
                TArray<UEdGraphPin*> InputPins;
                CastChecked<UComputeShaderGraphNode>(Node->GetGraphNode())->GetInputPins(/*out*/ InputPins);
                check(InputPins.Num() == Node->ChildNodes.Num());
                for (int32 ChildIndex = 0; ChildIndex < Node->ChildNodes.Num(); ChildIndex++)
                {
                    UComputeShaderNode* ChildNode = Node->ChildNodes[ChildIndex];
                    if (ChildNode)
                    {
                        InputPins[ChildIndex]->BreakAllPinLinks();
                        InputPins[ChildIndex]->MakeLinkTo(CastChecked<UComputeShaderGraphNode>(ChildNode->GetGraphNode())->GetOutputPin());
                    }
                }
            }
        }
    }

    void CompileFromGraphNodes(UComputeShader* ComputeShader) override
    {
        // Use GraphNodes to make ComputeShaderNode Connections
        TArray<UComputeShaderNode*> ChildNodes;
        TArray<UEdGraphPin*> InputPins;

        for (int32 NodeIndex = 0; NodeIndex < ComputeShader->ComputeShaderGraph->Nodes.Num(); ++NodeIndex)
        {
            UComputeShaderGraphNode* GraphNode = Cast<UComputeShaderGraphNode>(ComputeShader->ComputeShaderGraph->Nodes[NodeIndex]);
            if (GraphNode && GraphNode->ComputeShaderNode)
            {
                // Set ChildNodes
                GraphNode->GetInputPins(InputPins);
                ChildNodes.Empty();
                for (int32 PinIndex = 0; PinIndex < InputPins.Num(); ++PinIndex)
                {
                    UEdGraphPin* ChildPin = InputPins[PinIndex];

                    if (ChildPin->LinkedTo.Num() > 0)
                    {
                        UComputeShaderGraphNode* GraphChildNode = CastChecked<UComputeShaderGraphNode>(ChildPin->LinkedTo[0]->GetOwningNode());
                        ChildNodes.Add(GraphChildNode->ComputeShaderNode);
                    }
                    else
                    {
                        ChildNodes.AddZeroed();
                    }
                }

                GraphNode->ComputeShaderNode->SetFlags(RF_Transactional);
                GraphNode->ComputeShaderNode->Modify();
                GraphNode->ComputeShaderNode->SetChildNodes(ChildNodes);
                GraphNode->ComputeShaderNode->PostEditChange();
            }
            else
            {
                // Set FirstNode based on RootNode connection
                UComputeShaderGraphNode_Root* RootNode = Cast<UComputeShaderGraphNode_Root>(ComputeShader->ComputeShaderGraph->Nodes[NodeIndex]);
                if (RootNode)
                {
                    ComputeShader->Modify();
                    if (RootNode->Pins[0]->LinkedTo.Num() > 0)
                    {
                        ComputeShader->FirstNode = CastChecked<UComputeShaderGraphNode>(RootNode->Pins[0]->LinkedTo[0]->GetOwningNode())->ComputeShaderNode;
                    }
                    else
                    {
                        ComputeShader->FirstNode = nullptr;
                    }
                    ComputeShader->PostEditChange();
                }
            }
        }
    }

    void RemoveNullNodes(UComputeShader* ComputeShader) override
    {
        // Deal with ComputeShaderNode types being removed - iterate in reverse as nodes may be removed
        for (int32 idx = ComputeShader->ComputeShaderGraph->Nodes.Num() - 1; idx >= 0; --idx)
        {
            UComputeShaderGraphNode* Node = Cast<UComputeShaderGraphNode>(ComputeShader->ComputeShaderGraph->Nodes[idx]);

            if (Node && Node->ComputeShaderNode == nullptr)
            {
                FBlueprintEditorUtils::RemoveNode(nullptr, Node, true);
            }
        }
    }


    void CreateInputPin(UEdGraphNode* ComputeShaderGraphNode) override
    {
        CastChecked<UComputeShaderGraphNode>(ComputeShaderGraphNode)->CreateInputPin();
    }

    void RenameNodePins(UComputeShaderNode* ComputeShaderNode) override
    {
        TArray<class UEdGraphPin*> InputPins;

#if WITH_EDITORONLY_DATA
        CastChecked<UComputeShaderGraphNode>(ComputeShaderNode->GetGraphNode())->GetInputPins(InputPins);
#endif

        for (int32 i = 0; i < InputPins.Num(); i++)
        {
            if (InputPins[i])
            {
                InputPins[i]->PinName = *ComputeShaderNode->GetInputPinName(i).ToString();
            }
        }
    }

};


UComputeShaderEdGraph::UComputeShaderEdGraph(const FObjectInitializer& Initializer)
{
    
}

UComputeShader* UComputeShaderEdGraph::GetComputeShader() const
{
    return CastChecked<UComputeShader>(GetOuter());
}
