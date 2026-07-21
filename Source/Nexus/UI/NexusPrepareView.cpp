// Fill out your copyright notice in the Description page of Project Settings.

#include "NexusPrepareView.h"
#include "NexusAbilityUILibrary.h"
#include "NexusStashView.h"			// LogStashView (extern); reused so no duplicate log category
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NarrativeItem.h"

namespace
{
	const FName StashComponentName(TEXT("Stash"));
	const FName RunComponentName(TEXT("RunInventory"));
}

// ---------------------------------------------------------------------------------------------
// UNexusPrepareTile
// ---------------------------------------------------------------------------------------------

void UNexusPrepareTile::Init(UNexusPrepareView* InView, UNarrativeItem* InItem, bool bInStashSide)
{
	View = InView;
	Item = InItem;
	bStashSide = bInStashSide;
}

void UNexusPrepareTile::HandleClicked()
{
	if (View.IsValid() && Item.IsValid())
	{
		View->MoveTileItem(Item.Get(), bStashSide);
	}
}

// ---------------------------------------------------------------------------------------------
// UNexusPrepareView
// ---------------------------------------------------------------------------------------------

TSharedRef<SWidget> UNexusPrepareView::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildTree();
	}
	return Super::RebuildWidget();
}

void UNexusPrepareView::BuildTree()
{
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("PrepareRootCanvas"));
	WidgetTree->RootWidget = Canvas;

	// dimmed backdrop over the menu
	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Border_BG"));
	Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.9f));
	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Backdrop);
	PanelSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	PanelSlot->SetOffsets(FMargin(0.f));

	// title
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Title"));
	Title->SetText(NSLOCTEXT("NexusPrepare", "Title", "PREPARE FOR ROUND"));
	if (TitleFont.FontObject)
	{
		Title->SetFont(TitleFont);
	}
	PanelSlot = Canvas->AddChildToCanvas(Title);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.f));
	PanelSlot->SetPosition(FVector2D(0.f, 30.f));
	PanelSlot->SetAutoSize(true);

	// ---- LEFT column: STASH ----
	{
		UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Col_Stash"));

		UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_StashHeader"));
		Header->SetText(NSLOCTEXT("NexusPrepare", "StashHeader", "STASH"));
		if (LabelFont.FontObject) { Header->SetFont(LabelFont); }
		Col->AddChildToVerticalBox(Header);

		UTextBlock* Gold = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_StashGold"));
		Gold->SetText(NSLOCTEXT("NexusPrepare", "StashGold", "Gold: 0"));
		if (LabelFont.FontObject) { Gold->SetFont(LabelFont); }
		Gold->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.78f, 0.25f)));
		Col->AddChildToVerticalBox(Gold);

		UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("Scroll_Stash"));
		StashList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("List_Stash"));
		Scroll->AddChild(StashList);
		UVerticalBoxSlot* VSlot = Col->AddChildToVerticalBox(Scroll);
		VSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		PanelSlot = Canvas->AddChildToCanvas(Col);
		PanelSlot->SetAnchors(FAnchors(0.f, 0.f, 0.5f, 1.f));
		PanelSlot->SetOffsets(FMargin(60.f, 120.f, 30.f, 100.f));
	}

	// ---- RIGHT column: BRING (the basket) ----
	{
		UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Col_Basket"));

		UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_BasketHeader"));
		Header->SetText(NSLOCTEXT("NexusPrepare", "BasketHeader", "BRING (this run)"));
		if (LabelFont.FontObject) { Header->SetFont(LabelFont); }
		Col->AddChildToVerticalBox(Header);

		UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("Scroll_Basket"));
		BasketList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("List_Basket"));
		Scroll->AddChild(BasketList);
		UVerticalBoxSlot* VSlot = Col->AddChildToVerticalBox(Scroll);
		VSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		PanelSlot = Canvas->AddChildToCanvas(Col);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.f, 1.f, 1.f));
		PanelSlot->SetOffsets(FMargin(30.f, 120.f, 60.f, 100.f));
	}

	const bool bHaveButtonStyle =
		ButtonStyle.Normal.GetResourceObject() || ButtonStyle.Normal.TintColor != FSlateColor(FLinearColor::White);

	// back button (bottom-left)
	Button_Back = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Back"));
	if (bHaveButtonStyle) { Button_Back->SetStyle(ButtonStyle); }
	UTextBlock* BackLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Back"));
	BackLabel->SetText(NSLOCTEXT("NexusPrepare", "Back", "Back"));
	if (LabelFont.FontObject) { BackLabel->SetFont(LabelFont); }
	Button_Back->SetContent(BackLabel);
	PanelSlot = Canvas->AddChildToCanvas(Button_Back);
	PanelSlot->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
	PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
	PanelSlot->SetPosition(FVector2D(60.f, -40.f));
	PanelSlot->SetSize(FVector2D(240.f, 50.f));

	// launch button (bottom-right)
	Button_Launch = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Launch"));
	if (bHaveButtonStyle) { Button_Launch->SetStyle(ButtonStyle); }
	UTextBlock* LaunchLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Launch"));
	LaunchLabel->SetText(NSLOCTEXT("NexusPrepare", "Launch", "Launch"));
	if (LabelFont.FontObject) { LaunchLabel->SetFont(LabelFont); }
	Button_Launch->SetContent(LaunchLabel);
	PanelSlot = Canvas->AddChildToCanvas(Button_Launch);
	PanelSlot->SetAnchors(FAnchors(1.f, 1.f, 1.f, 1.f));
	PanelSlot->SetAlignment(FVector2D(1.f, 1.f));
	PanelSlot->SetPosition(FVector2D(-60.f, -40.f));
	PanelSlot->SetSize(FVector2D(240.f, 50.f));
}

void UNexusPrepareView::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Back)
	{
		Button_Back->OnClicked.AddUniqueDynamic(this, &UNexusPrepareView::HandleBackClicked);
	}
	if (Button_Launch)
	{
		Button_Launch->OnClicked.AddUniqueDynamic(this, &UNexusPrepareView::HandleLaunchClicked);
	}

	RefreshPanes();
}

void UNexusPrepareView::RebuildList(UVerticalBox* List, UNarrativeInventoryComponent* Source, bool bStashSide)
{
	if (!List)
	{
		return;
	}
	List->ClearChildren();
	if (!Source)
	{
		return;
	}

	const bool bHaveTileStyle =
		TileButtonStyle.Normal.GetResourceObject() || TileButtonStyle.Normal.TintColor != FSlateColor(FLinearColor::White);

	// GetItems returns a by-value snapshot, so nothing here can invalidate it.
	const TArray<UNarrativeItem*> Items = Source->GetItems();
	for (UNarrativeItem* Item : Items)
	{
		if (!IsValid(Item) || Item->GetQuantity() <= 0)
		{
			continue;
		}

		UButton* Tile = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		if (bHaveTileStyle) { Tile->SetStyle(TileButtonStyle); }

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		if (UTexture2D* Thumb = Item->Thumbnail.LoadSynchronous())
		{
			UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			Icon->SetBrushFromTexture(Thumb);
			Icon->SetDesiredSizeOverride(FVector2D(40.f, 40.f));
			UHorizontalBoxSlot* IconSlot = Row->AddChildToHorizontalBox(Icon);
			IconSlot->SetPadding(FMargin(4.f, 4.f, 8.f, 4.f));
			IconSlot->SetVerticalAlignment(VAlign_Center);
		}

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		const FText Name = Item->DisplayName.IsEmpty()
			? FText::FromString(Item->GetClass()->GetName())
			: Item->DisplayName;
		Label->SetText(Item->GetQuantity() > 1
			? FText::Format(NSLOCTEXT("NexusPrepare", "TileFmt", "{0}   x{1}"), Name, FText::AsNumber(Item->GetQuantity()))
			: Name);
		if (LabelFont.FontObject) { Label->SetFont(LabelFont); }
		UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label);
		LabelSlot->SetPadding(FMargin(4.f));
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		Tile->SetContent(Row);

		UNexusPrepareTile* Relay = NewObject<UNexusPrepareTile>(this);
		Relay->Init(this, Item, bStashSide);
		Tiles.Add(Relay);
		Tile->OnClicked.AddUniqueDynamic(Relay, &UNexusPrepareTile::HandleClicked);

		UVerticalBoxSlot* TileSlot = List->AddChildToVerticalBox(Tile);
		TileSlot->SetPadding(FMargin(2.f));
		TileSlot->SetHorizontalAlignment(HAlign_Fill);
	}
}

bool UNexusPrepareView::RefreshPanes()
{
	UNarrativeInventoryComponent* Stash =
		UNexusAbilityUILibrary::FindPlayerInventory(GetOwningPlayer(), StashComponentName);
	UNarrativeInventoryComponent* Run =
		UNexusAbilityUILibrary::FindPlayerInventory(GetOwningPlayer(), RunComponentName);
	if (!Stash || !Run)
	{
		UE_LOG(LogStashView, Warning,
			TEXT("PrepareView: missing %s (menu PlayerStateClass not BP_NexusPlayerState?)"),
			!Stash ? TEXT("Stash") : TEXT("RunInventory"));
		return false;
	}

	Tiles.Reset();
	RebuildList(StashList, Stash, /*bStashSide*/ true);
	RebuildList(BasketList, Run, /*bStashSide*/ false);

	if (UTextBlock* GoldText = WidgetTree ? WidgetTree->FindWidget<UTextBlock>(FName(TEXT("Text_StashGold"))) : nullptr)
	{
		GoldText->SetText(FText::Format(NSLOCTEXT("NexusPrepare", "StashGoldFmt", "Gold: {0}"),
			FText::AsNumber(Stash->GetCurrency())));
	}

	UE_LOG(LogStashView, Log, TEXT("PrepareView: refreshed (stash %d stack(s)/%d gold, basket %d stack(s))"),
		Stash->GetItems().Num(), Stash->GetCurrency(), Run->GetItems().Num());
	return true;
}

void UNexusPrepareView::MoveTileItem(UNarrativeItem* Item, bool bStashSide)
{
	if (!IsValid(Item))
	{
		return;
	}

	UNarrativeInventoryComponent* Stash =
		UNexusAbilityUILibrary::FindPlayerInventory(GetOwningPlayer(), StashComponentName);
	UNarrativeInventoryComponent* Run =
		UNexusAbilityUILibrary::FindPlayerInventory(GetOwningPlayer(), RunComponentName);
	if (!Stash || !Run)
	{
		return;
	}

	// stash tile -> into the basket; basket tile -> back to the stash
	UNarrativeInventoryComponent* Source = bStashSide ? Stash : Run;
	UNarrativeInventoryComponent* Dest = bStashSide ? Run : Stash;

	FText Error;
	const int32 Moved = UNexusAbilityUILibrary::MoveItemBetween(Source, Dest, Item, Error, /*Quantity*/ -1);
	if (Moved <= 0 && !Error.IsEmpty())
	{
		UE_LOG(LogStashView, Log, TEXT("PrepareView: move blocked: %s"), *Error.ToString());
	}

	RefreshPanes();
}

void UNexusPrepareView::LaunchRun()
{
	// Commit the basket first (stash -> NexusStash, basket -> NexusLoadout), then travel; the
	// in-run LoadRunLoadout splice reads NexusLoadout into RunInventory before the seeder runs.
	UNexusAbilityUILibrary::SaveStashAndLoadout(GetOwningPlayer());
	UGameplayStatics::OpenLevel(this, TravelLevelName, /*bAbsolute*/ true);
}

void UNexusPrepareView::HandleBackClicked()
{
	CloseView();
}

void UNexusPrepareView::HandleLaunchClicked()
{
	LaunchRun();
}

void UNexusPrepareView::CloseView()
{
	// Drop the temporary looting cross-links the moves set up, so the components don't sit with a
	// stale LootSource pointing at each other after the page closes.
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UNarrativeInventoryComponent* Stash = UNexusAbilityUILibrary::FindPlayerInventory(PC, StashComponentName))
		{
			Stash->SetLootSource(nullptr);
		}
		if (UNarrativeInventoryComponent* Run = UNexusAbilityUILibrary::FindPlayerInventory(PC, RunComponentName))
		{
			Run->SetLootSource(nullptr);
		}
	}
	RemoveFromParent();
}
