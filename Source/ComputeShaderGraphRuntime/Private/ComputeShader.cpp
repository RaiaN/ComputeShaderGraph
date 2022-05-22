#include "ComputeShader.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraphSchema.h"
#include "ComputeShaderNode.h"


UComputeShader::UComputeShader(const FObjectInitializer& Initializer)
: Super(Initializer)
{
    
}

#if WITH_EDITOR
static TSharedPtr<IComputeShaderEdGraphEditor> ComputeShaderEdGraphEditor;

void UComputeShader::PostInitProperties()
{
    Super::PostInitProperties();
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
    {
        CreateGraph();
    }
}

void UComputeShader::CompileFromGraphNodes()
{
    ensure (ComputeShaderEdGraphEditor.IsValid());
    ComputeShaderEdGraphEditor->CompileFromGraphNodes(this);
}

void UComputeShader::SetupComputeShaderNode(UComputeShaderNode* ComputeShaderNode, bool bSelectNewNode /*= true*/)
{
    // Create the graph node
    ensure(ComputeShaderEdGraphEditor.IsValid());
    check(ComputeShaderNode->GraphNode == NULL);

    ComputeShaderEdGraphEditor->SetupComputeShaderNode(ComputeShaderGraph, ComputeShaderNode, bSelectNewNode);
}

void UComputeShader::LinkGraphNodesFromComputeShaderNodes()
{
    ensure(ComputeShaderEdGraphEditor.IsValid());
    ComputeShaderEdGraphEditor->LinkGraphNodesFromComputeShaderNodes(this);
}

UEdGraph* UComputeShader::GetGraph()
{
    return ComputeShaderGraph;
}

void UComputeShader::CreateGraph()
{
    if (ComputeShaderGraph == nullptr)
    {
        ComputeShaderGraph = ComputeShaderEdGraphEditor->CreateNewComputeShaderGraph(this);
        ComputeShaderGraph->bAllowDeletion = false;

        // Give the schema a chance to fill out any required nodes (like the results node)
        const UEdGraphSchema* Schema = ComputeShaderGraph->GetSchema();
        Schema->CreateDefaultNodesForGraph(*ComputeShaderGraph);
    }
}

void UComputeShader::ClearGraph()
{
    if (ComputeShaderGraph)
    {
        ComputeShaderGraph->Nodes.Empty();
        // Give the schema a chance to fill out any required nodes (like the results node)
        const UEdGraphSchema* Schema = ComputeShaderGraph->GetSchema();
        Schema->CreateDefaultNodesForGraph(*ComputeShaderGraph);
    }
}

void UComputeShader::ResetGraph()
{
    for (const UComputeShaderNode* Node : AllNodes)
    {
        ComputeShaderGraph->RemoveNode(Node->GraphNode);
    }

    AllNodes.Reset();
    FirstNode = nullptr;
}

void UComputeShader::SetComputeShaderEdGraphEditor(TSharedPtr<IComputeShaderEdGraphEditor> InEditor)
{
    check(!ComputeShaderEdGraphEditor.IsValid());
    ComputeShaderEdGraphEditor = InEditor;
}

TSharedPtr<IComputeShaderEdGraphEditor> UComputeShader::GetComputeShaderEdGraphEditor()
{
    return ComputeShaderEdGraphEditor;
}

#endif