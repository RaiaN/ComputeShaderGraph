// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSGraphNode.h"
#include "ToolMenus.h"
#include "Editor/EditorEngine.h"
#include "CSEdGraph.h"
#include "GraphEditorActions.h"
#include "ScopedTransaction.h"
#include "Framework/Commands/GenericCommands.h"
#include "Engine/Font.h"
#include "ComputeShader.h"
#include "ComputeShaderNode.h"

#define LOCTEXT_NAMESPACE "CSGraphNode"

/////////////////////////////////////////////////////
// UComputeShaderGraphNode

void UComputeShaderGraphNode::PostLoad()
{
	Super::PostLoad();

	// Fixup any SoundNode back pointers that may be out of date
	if (ComputeShaderNode)
	{
		ComputeShaderNode->GraphNode = this;
	}

	for (int32 Index = 0; Index < Pins.Num(); ++Index)
	{
		UEdGraphPin* Pin = Pins[Index];
		if (Pin->PinName.IsNone())
		{
			// Makes sure pin has a name for lookup purposes but user will never see it
			if (Pin->Direction == EGPD_Input)
			{
				Pin->PinName = CreateUniquePinName(TEXT("Input"));
			}
			else
			{
				Pin->PinName = CreateUniquePinName(TEXT("Output"));
			}
			Pin->PinFriendlyName = FText::FromString(TEXT(" "));
		}
	}
}

void UComputeShaderGraphNode::SetComputeShaderNode(UComputeShaderNode* InComputeShaderNode)
{
	ComputeShaderNode = InComputeShaderNode;
	InComputeShaderNode->GraphNode = this;
}

void UComputeShaderGraphNode::CreateInputPin()
{
	UEdGraphPin* NewPin = CreatePin(EGPD_Input, TEXT("ComputeShaderNode"), *ComputeShaderNode->GetInputPinName(GetInputCount()).ToString());
	if (NewPin->PinName.IsNone())
	{
		// Makes sure pin has a name for lookup purposes but user will never see it
		NewPin->PinName = CreateUniquePinName(TEXT("Input"));
		NewPin->PinFriendlyName = FText::FromString(TEXT(" "));
	}
}

void UComputeShaderGraphNode::AddInputPin()
{
	const FScopedTransaction Transaction( NSLOCTEXT("ComputeShaderGraph", "ComputeShaderEditorAddInput", "Add Compute Shader Input") );
	Modify();
	CreateInputPin();

	UComputeShader* ComputeShader = CastChecked<UComputeShaderEdGraph>(GetGraph())->GetComputeShader();
	ComputeShader->CompileFromGraphNodes();
	ComputeShader->MarkPackageDirty();

	// Refresh the current graph, so the pins can be updated
	GetGraph()->NotifyGraphChanged();
}

void UComputeShaderGraphNode::RemoveInputPin(UEdGraphPin* InGraphPin)
{
	const FScopedTransaction Transaction( NSLOCTEXT("ComputeShaderGraph", "ComputeShaderEditorDeleteInput", "Delete Compute Shader Input") );
	Modify();

	TArray<class UEdGraphPin*> InputPins;
	GetInputPins(InputPins);

	for (int32 InputIndex = 0; InputIndex < InputPins.Num(); InputIndex++)
	{
		if (InGraphPin == InputPins[InputIndex])
		{
			InGraphPin->MarkPendingKill();
			Pins.Remove(InGraphPin);
			// also remove the SoundNode child node so ordering matches
			ComputeShaderNode->Modify();
			ComputeShaderNode->RemoveChildNode(InputIndex);
			break;
		}
	}

	UComputeShader* ComputeShader = CastChecked<UComputeShaderEdGraph>(GetGraph())->GetComputeShader();
	ComputeShader->CompileFromGraphNodes();
	ComputeShader->MarkPackageDirty();

	// Refresh the current graph, so the pins can be updated
	GetGraph()->NotifyGraphChanged();
}

int32 UComputeShaderGraphNode::EstimateNodeWidth() const
{
	const int32 EstimatedCharWidth = 6;
	FString NodeTitle = GetNodeTitle(ENodeTitleType::FullTitle).ToString();
	UFont* Font = GetDefault<UEditorEngine>()->EditorFont;
	int32 Result = NodeTitle.Len()*EstimatedCharWidth;

	if (Font)
	{
		Result = Font->GetStringSize(*NodeTitle);
	}

	return Result;
}

bool UComputeShaderGraphNode::CanAddInputPin() const
{
	if(ComputeShaderNode)
	{
		// Check if adding another input would exceed max child nodes.
		return ComputeShaderNode->ChildNodes.Num() < ComputeShaderNode->GetMaxChildNodes();
	}
	else
	{
		return false;
	}
}

FText UComputeShaderGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (ComputeShaderNode)
	{
		return ComputeShaderNode->GetTitle();
	}
	else
	{
		return Super::GetNodeTitle(TitleType);
	}
}

void UComputeShaderGraphNode::PrepareForCopying()
{
	if (ComputeShaderNode)
	{
		// Temporarily take ownership of the SoundNode, so that it is not deleted when cutting
		ComputeShaderNode->Rename(NULL, this, REN_DontCreateRedirectors);
	}
}

void UComputeShaderGraphNode::PostCopyNode()
{
	// Make sure the SoundNode goes back to being owned by the SoundCue after copying.
	ResetComputeShaderNodeOwner();
}

void UComputeShaderGraphNode::PostEditImport()
{
	// Make sure this SoundNode is owned by the SoundCue it's being pasted into.
	ResetComputeShaderNodeOwner();
}

void UComputeShaderGraphNode::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		CreateNewGuid();
	}
}

void UComputeShaderGraphNode::ResetComputeShaderNodeOwner()
{
	if (ComputeShaderNode)
	{
		UComputeShader* ComputeShader = CastChecked<UComputeShaderEdGraph>(GetGraph())->GetComputeShader();

		if (ComputeShaderNode->GetOuter() != ComputeShader)
		{
			// Ensures SoundNode is owned by the SoundCue
			ComputeShaderNode->Rename(NULL, ComputeShader, REN_DontCreateRedirectors);
		}

		// Set up the back pointer for newly created sound nodes
		ComputeShaderNode->GraphNode = this;
	}
}

void UComputeShaderGraphNode::CreateInputPins()
{
	if (ComputeShaderNode)
	{
		for (int32 ChildIndex = 0; ChildIndex < ComputeShaderNode->ChildNodes.Num(); ++ChildIndex)
		{
			CreateInputPin();
		}
	}
}

void UComputeShaderGraphNode::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin)
	{
		// If on an input that can be deleted, show option
		if(Context->Pin->Direction == EGPD_Input && ComputeShaderNode->ChildNodes.Num() > ComputeShaderNode->GetMinChildNodes())
		{
			FToolMenuSection& Section = Menu->AddSection("ComputeShaderGraphDeleteInput");

			// TODO
			// FIXME
			// Section.AddMenuEntry(FSoundCueGraphEditorCommands::Get().DeleteInput);
		}
	}
	else if (Context->Node)
	{
		{
			FToolMenuSection& Section = Menu->AddSection("ComputeShaderGraphNodeAlignment");
			Section.AddSubMenu("Alignment", LOCTEXT("AlignmentHeader", "Alignment"), FText(), FNewToolMenuDelegate::CreateLambda([](UToolMenu* SubMenu)
			{
				{
					FToolMenuSection& SubMenuSection = SubMenu->AddSection("EdGraphSchemaAlignment", LOCTEXT("AlignHeader", "Align"));
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesTop);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesMiddle);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesBottom);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesLeft);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesCenter);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesRight);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().StraightenConnections);
				}

				{
					FToolMenuSection& SubMenuSection = SubMenu->AddSection("EdGraphSchemaDistribution", LOCTEXT("DistributionHeader", "Distribution"));
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesHorizontally);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesVertically);
				}
			}));
		}

		{
			FToolMenuSection& Section = Menu->AddSection("ComputeShaderGraphNodeEdit");
			Section.AddMenuEntry( FGenericCommands::Get().Delete );
			Section.AddMenuEntry( FGenericCommands::Get().Cut );
			Section.AddMenuEntry( FGenericCommands::Get().Copy );
			Section.AddMenuEntry( FGenericCommands::Get().Duplicate );
		}

		{
			// TODO
			// FIXME
			/*FToolMenuSection& Section = Menu->AddSection("SoundCueGraphNodeAddPlaySync");
			if (CanAddInputPin())
			{
				Section.AddMenuEntry(FSoundCueGraphEditorCommands::Get().AddInput);
			}

			Section.AddMenuEntry(FSoundCueGraphEditorCommands::Get().PlayNode);

			if ( Cast<USoundNodeWavePlayer>(SoundNode) )
			{
				Section.AddMenuEntry(FSoundCueGraphEditorCommands::Get().BrowserSync);
			}
			else if (Cast<USoundNodeDialoguePlayer>(SoundNode))
			{
				Section.AddMenuEntry(FSoundCueGraphEditorCommands::Get().BrowserSync);
			}*/
		}
	}
}

FText UComputeShaderGraphNode::GetTooltipText() const
{
	FText Tooltip;
	if (ComputeShaderNode)
	{
		Tooltip = ComputeShaderNode->GetClass()->GetToolTipText();
	}
	if (Tooltip.IsEmpty())
	{
		Tooltip = GetNodeTitle(ENodeTitleType::ListView);
	}
	return Tooltip;
}

FString UComputeShaderGraphNode::GetDocumentationExcerptName() const
{
	// Default the node to searching for an excerpt named for the C++ node class name, including the U prefix.
	// This is done so that the excerpt name in the doc file can be found by find-in-files when searching for the full class name.
	UClass* MyClass = (ComputeShaderNode != NULL) ? ComputeShaderNode->GetClass() : this->GetClass();
	return FString::Printf(TEXT("%s%s"), MyClass->GetPrefixCPP(), *MyClass->GetName());
}

#undef LOCTEXT_NAMESPACE
