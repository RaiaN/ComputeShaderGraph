// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	CSGraphSchema.cpp
=============================================================================*/

#include "CSGraphSchema.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "Layout/SlateRect.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"
#include "EdGraphNode_Comment.h"
#include "EdGraph/EdGraph.h"
#include "CSEdGraph.h"
#include "CSGraphNode.h"
#include "CSGraphNode_Root.h"
#include "ComputeShader.h"
#include "ComputeShaderNode.h"
#include "GraphEditorActions.h"
#include "ScopedTransaction.h"
#include "GraphEditor.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "ComputeShaderGraphSchema"

TArray<UClass*> UComputeShaderGraphSchema::ComputeShaderNodeClasses;
bool UComputeShaderGraphSchema::bNodeClassesInitialized = false;

/////////////////////////////////////////////////////
// FSoundCueGraphSchemaAction_NewNode

UEdGraphNode* FComputeShaderGraphSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	check(ComputeShaderNodeClass);

	UComputeShader* ComputeShader = CastChecked<UComputeShaderEdGraph>(ParentGraph)->GetComputeShader();
	const FScopedTransaction Transaction( LOCTEXT("ComputeShaderGraphEditorNewSoundNode", "Compute Shader Graph Editor : New Compute Shader Node") );
	ParentGraph->Modify();
	ComputeShader->Modify();

	UComputeShaderNode* NewNode = ComputeShader->ConstructComputeShaderNode<UComputeShaderNode>(ComputeShaderNodeClass, bSelectNewNode);
	{
        // If this node allows >0 children but by default has zero - create a connector for starters
        if (NewNode->GetMaxChildNodes() > 0 && NewNode->ChildNodes.Num() == 0)
        {
            NewNode->CreateStartingConnectors();
        }

        // Attempt to connect inputs to selected nodes, unless we're already dragging from a single output
        if (FromPin == NULL || FromPin->Direction == EGPD_Input)
        {
            ConnectToSelectedNodes(NewNode, ParentGraph);
        }

        NewNode->GraphNode->NodePosX = Location.X;
        NewNode->GraphNode->NodePosY = Location.Y;

        NewNode->GraphNode->AutowireNewNode(FromPin);
	}

	ComputeShader->PostEditChange();
	ComputeShader->MarkPackageDirty();

	return NewNode->GraphNode;
}

void FComputeShaderGraphSchemaAction_NewNode::ConnectToSelectedNodes(UComputeShaderNode* NewNode, class UEdGraph* ParentGraph) const
{
	// only connect if node can have many children
	if (NewNode->GetMaxChildNodes() > 1)
	{
		// FIXME:
		// TODO:
		const FGraphPanelSelectionSet SelectedNodes; // = FSoundCueEditorUtilities::GetSelectedNodes(ParentGraph);

		TArray<UComputeShaderNode*> SortedNodes;
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UComputeShaderGraphNode* SelectedNode = Cast<UComputeShaderGraphNode>(*NodeIt);

			if (SelectedNode)
			{
				// Sort the nodes by y position
				bool bInserted = false;
				for (int32 Index = 0; Index < SortedNodes.Num(); ++Index)
				{
					if (SortedNodes[Index]->GraphNode->NodePosY > SelectedNode->NodePosY)
					{
						SortedNodes.Insert(SelectedNode->ComputeShaderNode, Index);
						bInserted = true;
						break;
					}
				}
				if (!bInserted)
				{
					SortedNodes.Add(SelectedNode->ComputeShaderNode);
				}
			}
		}
		if (SortedNodes.Num() > 1)
		{
			CastChecked<UComputeShaderGraphSchema>(NewNode->GraphNode->GetSchema())->TryConnectNodes(SortedNodes, NewNode);
		}
	}
}

bool UComputeShaderGraphSchema::ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const
{
	UComputeShaderGraphNode* InputNode = Cast<UComputeShaderGraphNode>(InputPin->GetOwningNode());

	if (InputNode)
	{
		// Only nodes representing SoundNodes have outputs
		UComputeShaderGraphNode* OutputNode = CastChecked<UComputeShaderGraphNode>(OutputPin->GetOwningNode());

		if (OutputNode->ComputeShaderNode)
		{
			// Grab all child nodes. We can't just test the output because 
			// the loop could happen from any additional child nodes. 
			TArray<UComputeShaderNode*> Nodes;
			OutputNode->ComputeShaderNode->GetAllNodes(Nodes);

			// If our test input is in that set, return true.
			return Nodes.Contains(InputNode->ComputeShaderNode);
		}
	}

	// Simple connection to root node
	return false;
}

void UComputeShaderGraphSchema::GetPaletteActions(FGraphActionMenuBuilder& ActionMenuBuilder) const
{
	GetAllSoundNodeActions(ActionMenuBuilder, false);
	GetCommentAction(ActionMenuBuilder);
}

void UComputeShaderGraphSchema::TryConnectNodes(const TArray<UComputeShaderNode*>& OutputNodes, UComputeShaderNode* InputNode) const
{
	for (int32 Index = 0; Index < OutputNodes.Num(); Index++)
	{
		if ( Index < InputNode->GetMaxChildNodes() )
		{
			UComputeShaderGraphNode* GraphNode = CastChecked<UComputeShaderGraphNode>(InputNode->GetGraphNode());
			if (Index >= GraphNode->GetInputCount())
			{
				GraphNode->CreateInputPin();
			}
			TryCreateConnection(GraphNode->GetInputPin(Index), CastChecked<UComputeShaderGraphNode>(OutputNodes[Index]->GetGraphNode())->GetOutputPin() );
		}
	}
}

void UComputeShaderGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	GetAllSoundNodeActions(ContextMenuBuilder, true);

	// GetCommentAction(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);

	/*if (!ContextMenuBuilder.FromPin && FSoundCueEditorUtilities::CanPasteNodes(ContextMenuBuilder.CurrentGraph))
	{
		TSharedPtr<FSoundCueGraphSchemaAction_Paste> NewAction( new FSoundCueGraphSchemaAction_Paste(FText::GetEmpty(), LOCTEXT("PasteHereAction", "Paste here"), FText::GetEmpty(), 0) );
		ContextMenuBuilder.AddAction( NewAction );
	}*/
}

void UComputeShaderGraphSchema::GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	if (Context->Node)
	{
		const UComputeShaderGraphNode* ComputeShaderGraphNode = Cast<const UComputeShaderGraphNode>(Context->Node);
		{
			FToolMenuSection& Section = Menu->AddSection("ComputeShaderGraphSchemaNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));
			Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
		}
	}

	Super::GetContextMenuActions(Menu, Context);
}

void UComputeShaderGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	const int32 RootNodeHeightOffset = -58;

	// Create the result node
	FGraphNodeCreator<UComputeShaderGraphNode_Root> NodeCreator(Graph);
	UComputeShaderGraphNode_Root* ResultRootNode = NodeCreator.CreateNode();
	ResultRootNode->NodePosY = RootNodeHeightOffset;
	NodeCreator.Finalize();
	SetNodeMetaData(ResultRootNode, FNodeMetadata::DefaultGraphNode);
}

const FPinConnectionResponse UComputeShaderGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionSameNode", "Both are on the same node"));
	}

	// Compare the directions
	const UEdGraphPin* InputPin = NULL;
	const UEdGraphPin* OutputPin = NULL;

	if (!CategorizePinsByDirection(PinA, PinB, /*out*/ InputPin, /*out*/ OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionIncompatible", "Directions are not compatible"));
	}

	if (ConnectionCausesLoop(InputPin, OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionLoop", "Connection would cause loop"));
	}

	// Break existing connections on inputs only - multiple output connections are acceptable
	if (InputPin->LinkedTo.Num() > 0)
	{
		ECanCreateConnectionResponse ReplyBreakOutputs;
		if (InputPin == PinA)
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_A;
		}
		else
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_B;
		}
		return FPinConnectionResponse(ReplyBreakOutputs, LOCTEXT("ConnectionReplace", "Replace existing connections"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

bool UComputeShaderGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	bool bModified = UEdGraphSchema::TryCreateConnection(PinA, PinB);

	if (bModified)
	{
        // TODO
        // FIXME
		// CastChecked<UComputeShaderEdGraph>(PinA->GetOwningNode()->GetGraph())->GetComputeShader()->CompileSoundNodesFromGraphNodes();
	}

	return bModified;
}

bool UComputeShaderGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const
{
	return true;
}

FLinearColor UComputeShaderGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor::Green;
}

void UComputeShaderGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	Super::BreakNodeLinks(TargetNode);
	
	// TODO
	// FIXME
	// CastChecked<UComputeShaderEdGraph>(TargetNode.GetGraph())->GetComputeShader()->CompileSoundNodesFromGraphNodes();
}

void UComputeShaderGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction( NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links") );

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

	// if this would notify the node then we need to compile the SoundCue
	if (bSendsNodeNotifcation)
	{
        // TODO
        // FIXME
		// CastChecked<USoundCueGraph>(TargetPin.GetOwningNode()->GetGraph())->GetSoundCue()->CompileSoundNodesFromGraphNodes();
	}
}

void UComputeShaderGraphSchema::GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const
{
	OutOkIcon = false;

	for (int32 AssetIdx = 0; AssetIdx < Assets.Num(); ++AssetIdx)
	{
		// As soon as one of the items is a sound wave, say we can drag it on... we actually eat only the sound waves.
		UComputeShader* ComputeShader = Cast<UComputeShader>(Assets[AssetIdx].GetAsset());
		if (ComputeShader)
		{
			OutOkIcon = true;
			break;
		}
	}
}

void UComputeShaderGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const
{
	/*//////////////////////////////////////////////////////////////////////////
	// Handle dropped USoundWaves
	TArray<USoundWave*> Waves;
	for (int32 AssetIdx = 0; AssetIdx < Assets.Num(); ++AssetIdx)
	{
		USoundWave* SoundWav = Cast<USoundWave>(Assets[AssetIdx].GetAsset());
		if (SoundWav)
		{
			Waves.Add(SoundWav);
		}
	}

	if (Waves.Num() > 0)
	{
		const FScopedTransaction Transaction( LOCTEXT("SoundCueEditorDropWave", "Sound Cue Editor: Drag and Drop Sound Wave") );

		USoundCueGraph* SoundCueGraph = CastChecked<USoundCueGraph>(Graph);
		USoundCue* SoundCue = SoundCueGraph->GetSoundCue();

		SoundCueGraph->Modify();

		TArray<USoundNode*> CreatedPlayers;
		FSoundCueEditorUtilities::CreateWaveContainers(Waves, SoundCue, CreatedPlayers, GraphPosition);
	}

	//////////////////////////////////////////////////////////////////////////
	// Handle dropped UDialogueWaves
	TArray<UDialogueWave*> Dialogues;
	for (int32 AssetIdx = 0; AssetIdx < Assets.Num(); ++AssetIdx)
	{
		UDialogueWave* DialogueWave = Cast<UDialogueWave>(Assets[AssetIdx].GetAsset());
		if (DialogueWave)
		{
			Dialogues.Add(DialogueWave);
		}
	}

	if (Dialogues.Num() > 0)
	{
		const FScopedTransaction Transaction(LOCTEXT("SoundCueEditorDropDialogue", "Sound Cue Editor: Drag and Drop Dialogue Wave"));

		USoundCueGraph* SoundCueGraph = CastChecked<USoundCueGraph>(Graph);
		USoundCue* SoundCue = SoundCueGraph->GetSoundCue();

		SoundCueGraph->Modify();

		TArray<USoundNode*> CreatedPlayers;
		FSoundCueEditorUtilities::CreateDialogueContainers(Dialogues, SoundCue, CreatedPlayers, GraphPosition);
	}*/
}

void UComputeShaderGraphSchema::DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const
{
	// Currently, drag and drop is only supported for dropping on sound cue graph nodes, and in particular, sound wave players and sound dialogue players.
	/*if (!Node->IsA<USoundCueGraphNode>())
	{
		return;
	}

	USoundCueGraphNode* SoundCueGraphNode = CastChecked<USoundCueGraphNode>(Node);
	USoundCueGraph* SoundCueGraph = CastChecked<USoundCueGraph>(Node->GetGraph());
	USoundCue* SoundCue = SoundCueGraph->GetSoundCue();

	TArray<USoundWave*> Waves;
	TArray<UDialogueWave*> Dialogues;
	for (int32 AssetIdx = 0; AssetIdx < Assets.Num(); ++AssetIdx)
	{
		USoundWave* SoundWav = Cast<USoundWave>(Assets[AssetIdx].GetAsset());
		if (SoundWav)
		{
			Waves.Add(SoundWav);
		}
		else
		{
			UDialogueWave* Dialogue = Cast<UDialogueWave>(Assets[AssetIdx].GetAsset());
			if (Dialogue)
			{
				Dialogues.Add(Dialogue);
			}
		}
	}

	USoundNodeWavePlayer* SoundNodeWavePlayer = Cast<USoundNodeWavePlayer>(SoundCueGraphNode->SoundNode);
	if (SoundNodeWavePlayer != nullptr)
	{
		if (Waves.Num() > 0)
		{
			if (Waves.Num() >= 1)
			{
				SoundCueGraph->Modify();
				SoundNodeWavePlayer->SetSoundWave(Waves[0]);
			}

			for (int32 Index = 1; Index < Waves.Num(); Index++)
			{
				TArray<USoundNode*> CreatedPlayers;
				FSoundCueEditorUtilities::CreateWaveContainers(Waves, SoundCue, CreatedPlayers, GraphPosition);
			}
		}
		else if (Dialogues.Num() > 0)
		{
			TArray<USoundNode*> CreatedPlayers;
			FSoundCueEditorUtilities::CreateDialogueContainers(Dialogues, SoundCue, CreatedPlayers, GraphPosition);

			if (CreatedPlayers.Num() > 0)
			{
				USoundNode* OldNode = SoundCueGraphNode->SoundNode;
				SoundCueGraphNode->SetSoundNode(CreatedPlayers[0]);

				// Make sure SoundCue is updated to match graph
				SoundCue->CompileSoundNodesFromGraphNodes();

				// Remove this node from the SoundCue's list of all SoundNodes
				SoundCue->AllNodes.Remove(OldNode);
				SoundCue->MarkPackageDirty();
			}
		}
	}

	USoundNodeDialoguePlayer* SoundNodeDialoguePlayer = Cast<USoundNodeDialoguePlayer>(SoundCueGraphNode->SoundNode);
	if (SoundNodeDialoguePlayer != nullptr)
	{
		if (Dialogues.Num() > 0)
		{
			if (Dialogues.Num() >= 1)
			{
				SoundCueGraph->Modify();
				SoundNodeDialoguePlayer->SetDialogueWave(Dialogues[0]);

				if (Dialogues[0]->ContextMappings.Num() == 1)
				{
					SoundNodeDialoguePlayer->DialogueWaveParameter.Context.Speaker = Dialogues[0]->ContextMappings[0].Context.Speaker;
					SoundNodeDialoguePlayer->DialogueWaveParameter.Context.Targets = Dialogues[0]->ContextMappings[0].Context.Targets;
				}
			}

			for (int32 Index = 1; Index < Waves.Num(); Index++)
			{
				TArray<USoundNode*> CreatedPlayers;
				FSoundCueEditorUtilities::CreateDialogueContainers(Dialogues, SoundCue, CreatedPlayers, GraphPosition);
			}
		}
		else if (Waves.Num() > 0)
		{
			TArray<USoundNode*> CreatedPlayers;
			FSoundCueEditorUtilities::CreateWaveContainers(Waves, SoundCue, CreatedPlayers, GraphPosition);

			if (CreatedPlayers.Num() > 0)
			{
				USoundNode* OldNode = SoundCueGraphNode->SoundNode;
				SoundCueGraphNode->SetSoundNode(CreatedPlayers[0]);

				// Make sure SoundCue is updated to match graph
				SoundCue->CompileSoundNodesFromGraphNodes();

				// Remove this node from the SoundCue's list of all SoundNodes
				SoundCue->AllNodes.Remove(OldNode);
				SoundCue->MarkPackageDirty();
			}
		}
	}

	SoundCueGraph->NotifyGraphChanged();*/
}

void UComputeShaderGraphSchema::GetAllSoundNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, bool bShowSelectedActions) const
{
	InitComputeShaderNodeClasses();

	FText SelectedItemText;
	bool IsSoundWaveSelected = false;
	bool IsDialogueWaveSelected = false;

	/*if (bShowSelectedActions)
	{
		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

		// Get display text for any items that may be selected
		if (ActionMenuBuilder.FromPin == NULL)
		{
			TArray<UComputeShader*> SelectedWavs;
			TArray<UDialogueWave*> SelectedDialogues;
			GEditor->GetSelectedObjects()->GetSelectedObjects<USoundWave>(SelectedWavs);
			GEditor->GetSelectedObjects()->GetSelectedObjects<UDialogueWave>(SelectedDialogues);

			int32 TotalWavs = SelectedWavs.Num() + SelectedDialogues.Num() ;

			if (TotalWavs > 1)
			{
				SelectedItemText = LOCTEXT("MultipleWAVsSelected", "Multiple WAVs");
			}
			else if (SelectedWavs.Num() == 1)
			{
				SelectedItemText = FText::FromString(SelectedWavs[0]->GetName());
				IsSoundWaveSelected = true;
			}
			else if (SelectedDialogues.Num() == 1)
			{
				SelectedItemText = FText::FromString(SelectedDialogues[0]->GetName());
				IsDialogueWaveSelected = true;
			}
		}
		else
		{
			USoundWave* SelectedWave = GEditor->GetSelectedObjects()->GetTop<USoundWave>();
			if (SelectedWave && ActionMenuBuilder.FromPin->Direction == EGPD_Input)
			{
				SelectedItemText = FText::FromString(SelectedWave->GetName());
				IsSoundWaveSelected = true;
			}
			else
			{
				UDialogueWave* SelectedDialogue = GEditor->GetSelectedObjects()->GetTop<UDialogueWave>();
				if (SelectedDialogue && ActionMenuBuilder.FromPin->Direction == EGPD_Input)
				{
					SelectedItemText = FText::FromString(SelectedDialogue->GetName());
					IsDialogueWaveSelected = true;
				}
			}
		}

		bShowSelectedActions = !SelectedItemText.IsEmpty();
	}*/

	for (UClass* ComputeShaderNodeClass : ComputeShaderNodeClasses)
	{
		UComputeShaderNode* ComputeShaderNode = ComputeShaderNodeClass->GetDefaultObject<UComputeShaderNode>();

		// when dragging from an output pin you can create anything but a wave player
		if (!ActionMenuBuilder.FromPin || ActionMenuBuilder.FromPin->Direction == EGPD_Input || ComputeShaderNode->GetMaxChildNodes() > 0)
		{
			const FText Name = FText::FromString(ComputeShaderNodeClass->GetDescription());

			{
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("Name"), Name);
				const FText AddToolTip = FText::Format(LOCTEXT("NewComputeShaderNodeTooltip", "Adds {Name} node here"), Arguments);
				TSharedPtr<FComputeShaderGraphSchemaAction_NewNode> NewNodeAction(new FComputeShaderGraphSchemaAction_NewNode(LOCTEXT("ComputeShaderNodeAction", "Compute Shader Node"), Name, AddToolTip, 0));
				ActionMenuBuilder.AddAction(NewNodeAction);
				NewNodeAction->ComputeShaderNodeClass = ComputeShaderNodeClass;
			}

			/*if (bShowSelectedActions &&
				(SoundNode->GetMaxChildNodes() == USoundNode::MAX_ALLOWED_CHILD_NODES || 
				   ((SoundNodeClass == USoundNodeWavePlayer::StaticClass() && IsSoundWaveSelected) || 
				    (SoundNodeClass == USoundNodeDialoguePlayer::StaticClass() && IsDialogueWaveSelected))))
			{
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("Name"), Name);
				Arguments.Add(TEXT("SelectedItems"), SelectedItemText);
				const FText MenuDesc = FText::Format(LOCTEXT("NewSoundNodeRandom", "{Name}: {SelectedItems}"), Arguments);
				const FText ToolTip = FText::Format(LOCTEXT("NewSoundNodeRandomTooltip", "Adds a {Name} node for {SelectedItems} here"), Arguments);
				TSharedPtr<FSoundCueGraphSchemaAction_NewFromSelected> NewNodeAction(new FSoundCueGraphSchemaAction_NewFromSelected(LOCTEXT("FromSelected", "From Selected"),
					MenuDesc,
					ToolTip, 0));
				ActionMenuBuilder.AddAction(NewNodeAction);
				NewNodeAction->SoundNodeClass = (SoundNodeClass == USoundNodeWavePlayer::StaticClass() || SoundNodeClass == USoundNodeDialoguePlayer::StaticClass() ? NULL : SoundNodeClass);
			}*/
		}
	}
}

void UComputeShaderGraphSchema::GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph) const
{
	/*if (!ActionMenuBuilder.FromPin)
	{
		const bool bIsManyNodesSelected = CurrentGraph ? (FSoundCueEditorUtilities::GetNumberOfSelectedNodes(CurrentGraph) > 0) : false;
		const FText MenuDescription = bIsManyNodesSelected ? LOCTEXT("CreateCommentAction", "Create Comment from Selection") : LOCTEXT("AddCommentAction", "Add Comment...");
		const FText ToolTip = LOCTEXT("CreateCommentToolTip", "Creates a comment.");

		TSharedPtr<FSoundCueGraphSchemaAction_NewComment> NewAction(new FSoundCueGraphSchemaAction_NewComment(FText::GetEmpty(), MenuDescription, ToolTip, 0));
		ActionMenuBuilder.AddAction( NewAction );
	}*/
}

void UComputeShaderGraphSchema::InitComputeShaderNodeClasses()
{
	if(bNodeClassesInitialized)
	{
		return;
	}

	// Construct list of non-abstract sound node classes.
	for(TObjectIterator<UClass> It; It; ++It)
	{
		if(It->IsChildOf(UComputeShaderNode::StaticClass()) 
			&& !It->HasAnyClassFlags(CLASS_Abstract))
		{
			ComputeShaderNodeClasses.Add(*It);
		}
	}

	ComputeShaderNodeClasses.Sort();

	bNodeClassesInitialized = true;
}

int32 UComputeShaderGraphSchema::GetNodeSelectionCount(const UEdGraph* Graph) const
{
	// TODO
	// FIXME
	return 0; // FSoundCueEditorUtilities::GetNumberOfSelectedNodes(Graph);
}

/*TSharedPtr<FEdGraphSchemaAction> UComputeShaderGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FSoundCueGraphSchemaAction_NewComment));
}*/

#undef LOCTEXT_NAMESPACE
