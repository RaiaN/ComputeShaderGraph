// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EdGraph/EdGraphNode.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "ScopedTransaction.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphUtilities.h"
#include "SNodePanel.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/GenericCommands.h"
#include "HAL/PlatformApplicationMisc.h"
#include "CSEdGraph.h"
#include "ComputeShaderGraph/CSGraphNode.h"
#include "ComputeShaderGraph/CSGraphNode_Root.h"
#include "ComputeShaderGraph/CSGraphSchema.h"
#include "ComputeShader.h"
#include "ComputeShaderNode.h"
#include "Palette/SComputeShaderPalette.h"

#define LOCTEXT_NAMESPACE "ComputeShaderEditor"

const FName FComputeShaderEditor::GraphCanvasTabId( TEXT( "ComputeShaderEditor_GraphCanvas" ) );
const FName FComputeShaderEditor::PropertiesTabId( TEXT( "ComputeShaderEditor_Properties" ) );
const FName FComputeShaderEditor::PaletteTabId( TEXT( "ComputeShaderEditor_Palette" ) );

FComputeShaderEditor::FComputeShaderEditor()
	: ComputeShader(nullptr)
{
}

void FComputeShaderEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_SoundCueEditor", "Sound Cue Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner( GraphCanvasTabId, FOnSpawnTab::CreateSP(this, &FComputeShaderEditor::SpawnTab_GraphCanvas) )
		.SetDisplayName( LOCTEXT("GraphCanvasTab", "Viewport") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner( PropertiesTabId, FOnSpawnTab::CreateSP(this, &FComputeShaderEditor::SpawnTab_Properties) )
		.SetDisplayName( LOCTEXT("DetailsTab", "Details") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PaletteTabId, FOnSpawnTab::CreateSP(this, &FComputeShaderEditor::SpawnTab_Palette))
		.SetDisplayName( LOCTEXT("PaletteTab", "Palette") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Palette"));
}

void FComputeShaderEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner( GraphCanvasTabId );
	InTabManager->UnregisterTabSpawner( PropertiesTabId );
	InTabManager->UnregisterTabSpawner( PaletteTabId );
}

FComputeShaderEditor::~FComputeShaderEditor()
{
	
}

void FComputeShaderEditor::InitComputeShaderEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit)
{
	ComputeShader = CastChecked<UComputeShader>(ObjectToEdit);

	// Support undo/redo
	ComputeShader->SetFlags(RF_Transactional);
	
	GEditor->RegisterForUndo(this);

	FGraphEditorCommands::Register();

	// FIXME
	// TODO
	// FSoundCueGraphEditorCommands::Register();

	BindGraphCommands();

	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_ComputeShaderEditor_Layout_v1")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.1f)
			->SetHideTabWell(true)
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
		->Split(FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)
			->SetSizeCoefficient(0.9f)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.225f)
				->SetHideTabWell(true)
				->AddTab(PropertiesTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.65f)
				->SetHideTabWell(true)
				->AddTab(GraphCanvasTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.125f)
				->SetHideTabWell(true)
				->AddTab(PaletteTabId, ETabState::OpenedTab)
			)
		)
	);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, TEXT("ComputeShaderEditorApp"), StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit, false);

	// FIXME 
	// TODO
	// IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>( "AudioEditor" );
	// AddMenuExtender(AudioEditorModule->GetSoundCueMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	ExtendToolbar();
	RegenerateMenusAndToolbars();
	
	// @todo toolkit world centric editing
	/*if(IsWorldCentricAssetEditor())
	{
		SpawnToolkitTab(GetToolbarTabId(), FString(), EToolkitTabSpot::ToolBar);
		SpawnToolkitTab(GraphCanvasTabId, FString(), EToolkitTabSpot::Viewport);
		SpawnToolkitTab(PropertiesTabId, FString(), EToolkitTabSpot::Details);
	}*/
}

UComputeShader* FComputeShaderEditor::GetComputeShader() const
{
	return ComputeShader;
}

void FComputeShaderEditor::SetSelection(TArray<UObject*> SelectedObjects)
{
	if (ComputeShaderProperties.IsValid())
	{
		ComputeShaderProperties->SetObjects(SelectedObjects);
	}
}

bool FComputeShaderEditor::GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding )
{
	return CSGraphEditor->GetBoundsForSelectedNodes(Rect, Padding);
}

int32 FComputeShaderEditor::GetNumberOfSelectedNodes() const
{
	return CSGraphEditor->GetSelectedNodes().Num();
}

FName FComputeShaderEditor::GetToolkitFName() const
{
	return FName("ComputeShaderEditor");
}

FText FComputeShaderEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "ComputeShader Editor");
}

FString FComputeShaderEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "ComputeShader ").ToString();
}

FLinearColor FComputeShaderEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

TSharedRef<SDockTab> FComputeShaderEditor::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args)
{
	check( Args.GetTabId() == GraphCanvasTabId );

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ComputeShaderGraphCanvasTitle", "Viewport"));

	if (CSGraphEditor.IsValid())
	{
		SpawnedTab->SetContent(CSGraphEditor.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FComputeShaderEditor::SpawnTab_Properties(const FSpawnTabArgs& Args)
{
	check( Args.GetTabId() == PropertiesTabId );

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("ComputeShaderDetailsTitle", "Details"))
		[
			ComputeShaderProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab> FComputeShaderEditor::SpawnTab_Palette(const FSpawnTabArgs& Args)
{
	check( Args.GetTabId() == PaletteTabId );

	return SNew(SDockTab)
	    .Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
		.Label(LOCTEXT("ComputeShaderPaletteTitle", "Palette"))
		[
			Palette.ToSharedRef()
		];
}

void FComputeShaderEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ComputeShader);
}

void FComputeShaderEditor::PostUndo(bool bSuccess)
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->ClearSelectionSet();
		CSGraphEditor->NotifyGraphChanged();
		FSlateApplication::Get().DismissAllMenus();
	}

}

void FComputeShaderEditor::NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, class FProperty* PropertyThatChanged)
{
	if (CSGraphEditor.IsValid() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		CSGraphEditor->NotifyGraphChanged();
	}
}

void FComputeShaderEditor::CreateInternalWidgets()
{
	CSGraphEditor = CreateGraphEditorWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	ComputeShaderProperties = PropertyModule.CreateDetailView(Args);
	ComputeShaderProperties->SetObject( ComputeShader );

	Palette = SNew(SComputeShaderPalette);
}

void FComputeShaderEditor::ExtendToolbar()
{
	struct Local
{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			// TODO
			// FIXME
		}
	};

	/*TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic( &Local::FillToolbar )
		);

	AddToolbarExtender(ToolbarExtender);*/

	// IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>( "AudioEditor" );
	// AddToolbarExtender(AudioEditorModule->GetSoundCueToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
}

void FComputeShaderEditor::BindGraphCommands()
{
	// const FSoundCueGraphEditorCommands& Commands = FSoundCueGraphEditorCommands::Get();

	/*ToolkitCommands->MapAction(
		Commands.PlayCue,
		FExecuteAction::CreateSP(this, &FComputeShaderEditor::PlayCue));

	ToolkitCommands->MapAction(
		Commands.PlayNode,
		FExecuteAction::CreateSP(this, &FComputeShaderEditor::PlayNode),
		FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanPlayNode ));

	ToolkitCommands->MapAction(
		Commands.StopCueNode,
		FExecuteAction::CreateSP(this, &FComputeShaderEditor::Stop));

	ToolkitCommands->MapAction(
		Commands.TogglePlayback,
		FExecuteAction::CreateSP(this, &FComputeShaderEditor::TogglePlayback));

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Undo,
		FExecuteAction::CreateSP( this, &FComputeShaderEditor::UndoGraphAction ));

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Redo,
		FExecuteAction::CreateSP( this, &FComputeShaderEditor::RedoGraphAction ));

	ToolkitCommands->MapAction(
		Commands.ToggleSolo,
		FExecuteAction::CreateSP(this, &FComputeShaderEditor::ToggleSolo),
		FCanExecuteAction::CreateSP(this, &FComputeShaderEditor::CanExcuteToggleSolo),
		FIsActionChecked::CreateSP(this, &FComputeShaderEditor::IsSoloToggled));
		
	ToolkitCommands->MapAction(
		Commands.ToggleMute,
		FExecuteAction::CreateSP(this, &FComputeShaderEditor::ToggleMute),
		FCanExecuteAction::CreateSP(this, &FComputeShaderEditor::CanExcuteToggleMute),
		FIsActionChecked::CreateSP(this, &FComputeShaderEditor::IsMuteToggled));*/
}

void FComputeShaderEditor::SyncInBrowser()
{
	TArray<UObject*> ObjectsToSync;
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	/*for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UComputeShaderGraphNode* SelectedNode = Cast<UComputeShaderGraphNode>(*NodeIt);

		if (SelectedNode)
		{
			USoundNodeWavePlayer* SelectedWave = Cast<USoundNodeWavePlayer>(SelectedNode->ComputeShaderNode);
			if (SelectedWave && SelectedWave->GetSoundWave())
			{
				ObjectsToSync.AddUnique(SelectedWave->GetSoundWave());
			}

			USoundNodeDialoguePlayer* SelectedDialogue = Cast<USoundNodeDialoguePlayer>(SelectedNode->ComputeShaderNode);
			if (SelectedDialogue && SelectedDialogue->GetDialogueWave())
			{
				ObjectsToSync.AddUnique(SelectedDialogue->GetDialogueWave());
			}
		}
	}*/

	if (ObjectsToSync.Num() > 0)
	{
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}
}

bool FComputeShaderEditor::CanSyncInBrowser() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	/*for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UComputeShaderGraphNode* SelectedNode = Cast<UComputeShaderGraphNode>(*NodeIt);

		if (SelectedNode)
		{
			USoundNodeWavePlayer* WavePlayer = Cast<USoundNodeWavePlayer>(SelectedNode->ComputeShaderNode);

			if (WavePlayer && WavePlayer->GetSoundWave())
			{
				return true;
			}

			USoundNodeDialoguePlayer* SelectedDialogue = Cast<USoundNodeDialoguePlayer>(SelectedNode->ComputeShaderNode);
			if (SelectedDialogue && SelectedDialogue->GetDialogueWave())
			{
				return true;
			}
		}
	}*/
	return false;
}

void FComputeShaderEditor::AddInput()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	// Iterator used but should only contain one node
	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UComputeShaderGraphNode* SelectedNode = Cast<UComputeShaderGraphNode>(*NodeIt);

		if (SelectedNode)
		{
			SelectedNode->AddInputPin();
			break;
		}
	}
}

bool FComputeShaderEditor::CanAddInput() const
{
	return GetSelectedNodes().Num() == 1;
}

void FComputeShaderEditor::DeleteInput()
{
	if (CSGraphEditor.IsValid())
	{
		UEdGraphPin* SelectedPin = CSGraphEditor->GetGraphPinForMenu();
		if (ensure(SelectedPin))
		{
			UComputeShaderGraphNode* SelectedNode = Cast<UComputeShaderGraphNode>(SelectedPin->GetOwningNode());

			if (SelectedNode && SelectedNode == SelectedPin->GetOwningNode())
			{
				SelectedNode->RemoveInputPin(SelectedPin);
			}
		}
	}
}

bool FComputeShaderEditor::CanDeleteInput() const
{
	return true;
}

void FComputeShaderEditor::OnCreateComment()
{
	// FSoundCueGraphSchemaAction_NewComment CommentAction;
	// CommentAction.PerformAction(ComputeShader->SoundCueGraph, NULL, CSGraphEditor->GetPasteLocation());
}

TSharedRef<SGraphEditor> FComputeShaderEditor::CreateGraphEditorWidget()
{
	if ( !GraphEditorCommands.IsValid() )
	{
		GraphEditorCommands = MakeShareable( new FUICommandList );

		/*GraphEditorCommands->MapAction(FSoundCueGraphEditorCommands::Get().PlayNode,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::PlayNode),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanPlayNode ));

		GraphEditorCommands->MapAction( FSoundCueGraphEditorCommands::Get().BrowserSync,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::SyncInBrowser),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanSyncInBrowser ));

		GraphEditorCommands->MapAction( FSoundCueGraphEditorCommands::Get().AddInput,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::AddInput),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanAddInput ));

		GraphEditorCommands->MapAction( FSoundCueGraphEditorCommands::Get().DeleteInput,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::DeleteInput),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanDeleteInput ));

		// Graph Editor Commands
		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().CreateComment,
			FExecuteAction::CreateSP( this, &FComputeShaderEditor::OnCreateComment )
			);
		*/

		// Editing commands
		GraphEditorCommands->MapAction( FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateSP( this, &FComputeShaderEditor::SelectAllNodes ),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanSelectAllNodes )
			);

		GraphEditorCommands->MapAction( FGenericCommands::Get().Delete,
			FExecuteAction::CreateSP( this, &FComputeShaderEditor::DeleteSelectedNodes ),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanDeleteNodes )
			);

		GraphEditorCommands->MapAction( FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP( this, &FComputeShaderEditor::CopySelectedNodes ),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanCopyNodes )
			);

		GraphEditorCommands->MapAction( FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP( this, &FComputeShaderEditor::CutSelectedNodes ),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanCutNodes )
			);

		GraphEditorCommands->MapAction( FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP( this, &FComputeShaderEditor::PasteNodes ),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanPasteNodes )
			);

		GraphEditorCommands->MapAction( FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP( this, &FComputeShaderEditor::DuplicateNodes ),
			FCanExecuteAction::CreateSP( this, &FComputeShaderEditor::CanDuplicateNodes )
			);

		// Alignment Commands
		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().AlignNodesTop,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnAlignTop)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().AlignNodesMiddle,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnAlignMiddle)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().AlignNodesBottom,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnAlignBottom)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().AlignNodesLeft,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnAlignLeft)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().AlignNodesCenter,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnAlignCenter)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().AlignNodesRight,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnAlignRight)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().StraightenConnections,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnStraightenConnections)
		);

		// Distribution Commands
		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().DistributeNodesHorizontally,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnDistributeNodesH)
		);

		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().DistributeNodesVertically,
			FExecuteAction::CreateSP(this, &FComputeShaderEditor::OnDistributeNodesV)
		);
	}

	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_SoundCue", "SOUND CUE");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FComputeShaderEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FComputeShaderEditor::OnNodeTitleCommitted);
	// InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FComputeShaderEditor::PlaySingleNode);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(ComputeShader->GetGraph())
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

FGraphPanelSelectionSet FComputeShaderEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	if (CSGraphEditor.IsValid())
	{
		CurrentSelection = CSGraphEditor->GetSelectedNodes();
	}
	return CurrentSelection;
}

void FComputeShaderEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	if(NewSelection.Num())
	{
		for(TSet<class UObject*>::TConstIterator SetIt(NewSelection);SetIt;++SetIt)
		{
			if (Cast<UComputeShaderGraphNode_Root>(*SetIt))
			{
				Selection.Add(GetComputeShader());
			}
			else if (UComputeShaderGraphNode* GraphNode = Cast<UComputeShaderGraphNode>(*SetIt))
			{
				Selection.Add(GraphNode->ComputeShaderNode);
			}
			else
			{
				Selection.Add(*SetIt);
			}
		}
		//Selection = NewSelection.Array();
	}
	else
	{
		Selection.Add(GetComputeShader());
	}

	SetSelection(Selection);
}

void FComputeShaderEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction( LOCTEXT( "RenameNode", "Rename Node" ) );
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

void FComputeShaderEditor::SelectAllNodes()
{
	CSGraphEditor->SelectAllNodes();
}

bool FComputeShaderEditor::CanSelectAllNodes() const
{
	return true;
}

void FComputeShaderEditor::DeleteSelectedNodes()
{
	const FScopedTransaction Transaction( NSLOCTEXT("UnrealEd", "SoundCueEditorDeleteSelectedNode", "Delete Selected Sound Cue Node") );

	CSGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	CSGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* Node = CastChecked<UEdGraphNode>(*NodeIt);

		if (Node->CanUserDeleteNode())
		{
			if (UComputeShaderGraphNode* ComputeShaderGraphNode = Cast<UComputeShaderGraphNode>(Node))
			{
				UComputeShaderNode* DelNode = ComputeShaderGraphNode->ComputeShaderNode;

				FBlueprintEditorUtils::RemoveNode(NULL, ComputeShaderGraphNode, true);

				// Make sure ComputeShader is updated to match graph
				ComputeShader->CompileFromGraphNodes();

				// Remove this node from the ComputeShader's list of all SoundNodes
				ComputeShader->AllNodes.Remove(DelNode);
				ComputeShader->MarkPackageDirty();
			}
			else
			{
				FBlueprintEditorUtils::RemoveNode(NULL, Node, true);
			}
		}
	}
}

bool FComputeShaderEditor::CanDeleteNodes() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	if (SelectedNodes.Num() == 1)
	{
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			if (Cast<UComputeShaderGraphNode_Root>(*NodeIt))
			{
				// Return false if only root node is selected, as it can't be deleted
				return false;
			}
		}
	}

	return SelectedNodes.Num() > 0;
}

void FComputeShaderEditor::DeleteSelectedDuplicatableNodes()
{
	// Cache off the old selection
	const FGraphPanelSelectionSet OldSelectedNodes = GetSelectedNodes();

	// Clear the selection and only select the nodes that can be duplicated
	FGraphPanelSelectionSet RemainingNodes;
	CSGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			CSGraphEditor->SetNodeSelection(Node, true);
		}
		else
		{
			RemainingNodes.Add(Node);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	// Reselect whatever's left from the original selection after the deletion
	CSGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(RemainingNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			CSGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FComputeShaderEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	// Cut should only delete nodes that can be duplicated
	DeleteSelectedDuplicatableNodes();
}

bool FComputeShaderEditor::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FComputeShaderEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		if(UComputeShaderGraphNode* Node = Cast<UComputeShaderGraphNode>(*SelectedIter))
		{
			Node->PrepareForCopying();
		}
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	// Make sure ComputeShader remains the owner of the copied nodes
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UComputeShaderGraphNode* Node = Cast<UComputeShaderGraphNode>(*SelectedIter))
		{
			Node->PostCopyNode();
		}
	}
}

bool FComputeShaderEditor::CanCopyNodes() const
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			return true;
		}
	}
	return false;
}

void FComputeShaderEditor::PasteNodes()
{
	PasteNodesHere(CSGraphEditor->GetPasteLocation());
}

void FComputeShaderEditor::PasteNodesHere(const FVector2D& Location)
{
	// Undo/Redo support
	const FScopedTransaction Transaction( NSLOCTEXT("UnrealEd", "SoundCueEditorPaste", "Paste Sound Cue Node") );
	ComputeShader->GetGraph()->Modify();
	ComputeShader->Modify();

	// Clear the selection set (newly pasted stuff will be selected)
	CSGraphEditor->ClearSelectionSet();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(ComputeShader->GetGraph(), TextToImport, /*out*/ PastedNodes);

	//Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f,0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	if ( PastedNodes.Num() > 0 )
	{
		float InvNumNodes = 1.0f/float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;

		if (UComputeShaderGraphNode* ComputeShaderGraphNode = Cast<UComputeShaderGraphNode>(Node))
		{
			ComputeShader->AllNodes.Add(ComputeShaderGraphNode->ComputeShaderNode);
		}

		// Select the newly pasted stuff
		CSGraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X ;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y ;

		Node->SnapToGrid(SNodePanel::GetSnapGridSize());

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	// Force new pasted ComputeShaderNodes to have same connections as graph nodes
	ComputeShader->CompileFromGraphNodes();

	// Update UI
	CSGraphEditor->NotifyGraphChanged();

	ComputeShader->PostEditChange();
	ComputeShader->MarkPackageDirty();
}

bool FComputeShaderEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(ComputeShader->ComputeShaderGraph, ClipboardContent);
}

void FComputeShaderEditor::DuplicateNodes()
{
	// Copy and paste current selection
	CopySelectedNodes();
	PasteNodes();
}

bool FComputeShaderEditor::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

void FComputeShaderEditor::UndoGraphAction()
{
	GEditor->UndoTransaction();
}

void FComputeShaderEditor::RedoGraphAction()
{
	// Clear selection, to avoid holding refs to nodes that go away
	CSGraphEditor->ClearSelectionSet();

	GEditor->RedoTransaction();
}

void FComputeShaderEditor::OnAlignTop()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnAlignTop();
	}
}

void FComputeShaderEditor::OnAlignMiddle()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnAlignMiddle();
	}
}

void FComputeShaderEditor::OnAlignBottom()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnAlignBottom();
	}
}

void FComputeShaderEditor::OnAlignLeft()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnAlignLeft();
	}
}

void FComputeShaderEditor::OnAlignCenter()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnAlignCenter();
	}
}

void FComputeShaderEditor::OnAlignRight()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnAlignRight();
	}
}

void FComputeShaderEditor::OnStraightenConnections()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnStraightenConnections();
	}
}

void FComputeShaderEditor::OnDistributeNodesH()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnDistributeNodesH();
	}
}

void FComputeShaderEditor::OnDistributeNodesV()
{
	if (CSGraphEditor.IsValid())
	{
		CSGraphEditor->OnDistributeNodesV();
	}
}

#undef LOCTEXT_NAMESPACE
