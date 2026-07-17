// Fill out your copyright notice in the Description page of Project Settings.

#include "NexusStashView.h"
#include "NexusAbilityUILibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "InventoryComponent.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY(LogStashView);

TSharedRef<SWidget> UNexusStashView::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildTree();
	}
	return Super::RebuildWidget();
}

void UNexusStashView::BuildTree()
{
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("StashRootCanvas"));
	WidgetTree->RootWidget = Canvas;

	// dimmed backdrop over the menu
	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Border_BG"));
	Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.85f));
	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Backdrop);
	PanelSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	PanelSlot->SetOffsets(FMargin(0.f));

	// title
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Title"));
	Title->SetText(NSLOCTEXT("NexusStash", "StashTitle", "STASH"));
	if (TitleFont.FontObject)
	{
		Title->SetFont(TitleFont);
	}
	PanelSlot = Canvas->AddChildToCanvas(Title);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.f));
	PanelSlot->SetPosition(FVector2D(0.f, 40.f));
	PanelSlot->SetAutoSize(true);

	// the plugin loot pane
	if (StashPanelClass)
	{
		StashPanel = CreateWidget<UUserWidget>(this, StashPanelClass, TEXT("StashPanel"));
		PanelSlot = Canvas->AddChildToCanvas(StashPanel);
		// Stretch vertically between the title strip and the back button. The pane used to
		// be a fixed 900x720 dead-centered, so on viewports shorter than ~850px its top
		// edge (H/2 - 340) rose above the title at y=40 and "STASH" drew over the pane's box.
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 1.f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.f));
		PanelSlot->SetOffsets(FMargin(0.f, 130.f, 900.f, 110.f));
	}
	else
	{
		UE_LOG(LogStashView, Warning, TEXT("StashView: StashPanelClass unset on the CDO"));
	}

	// currency readout — the plugin loot pane renders items only, so gold gets its own
	// line, right-aligned to the pane's top-right corner. Filled from the real Stash
	// component in InitializeStashPanel; found back by name there (no UPROPERTY on
	// purpose: keeps this a .cpp-only change, safe to Live-Coding patch).
	UTextBlock* GoldText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Gold"));
	GoldText->SetText(NSLOCTEXT("NexusStash", "StashGold", "Gold: 0"));
	FSlateFontInfo GoldFont = LabelFont.FontObject ? LabelFont : TitleFont;
	if (GoldFont.FontObject)
	{
		GoldFont.Size = 22;
		GoldText->SetFont(GoldFont);
	}
	GoldText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.78f, 0.25f)));
	PanelSlot = Canvas->AddChildToCanvas(GoldText);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
	PanelSlot->SetAlignment(FVector2D(1.f, 0.f));
	PanelSlot->SetPosition(FVector2D(450.f, 96.f));
	PanelSlot->SetAutoSize(true);

	// back button
	Button_Back = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Back"));
	if (BackButtonStyle.Normal.GetResourceObject() || BackButtonStyle.Normal.TintColor != FSlateColor(FLinearColor::White))
	{
		Button_Back->SetStyle(BackButtonStyle);
	}
	UTextBlock* BackLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Back"));
	BackLabel->SetText(NSLOCTEXT("NexusStash", "StashBack", "Back"));
	if (LabelFont.FontObject)
	{
		BackLabel->SetFont(LabelFont);
	}
	Button_Back->SetContent(BackLabel);
	PanelSlot = Canvas->AddChildToCanvas(Button_Back);
	PanelSlot->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 1.f));
	PanelSlot->SetPosition(FVector2D(0.f, -40.f));
	PanelSlot->SetSize(FVector2D(280.f, 50.f));
}

void UNexusStashView::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Back)
	{
		Button_Back->OnClicked.AddUniqueDynamic(this, &UNexusStashView::HandleBackClicked);
	}

	InitializeStashPanel();
}

void UNexusStashView::HandleBackClicked()
{
	CloseStashView();
}

void UNexusStashView::CloseStashView()
{
	RemoveFromParent();
}

bool UNexusStashView::InitializeStashPanel()
{
	UNarrativeInventoryComponent* Stash =
		UNexusAbilityUILibrary::FindPlayerInventory(GetOwningPlayer(), FName(TEXT("Stash")));
	if (!StashPanel || !Stash)
	{
		UE_LOG(LogStashView, Warning, TEXT("StashView: %s"),
			!StashPanel ? TEXT("no StashPanel built (StashPanelClass unset?)")
			            : TEXT("no Stash component (menu PlayerStateClass not BP_NexusPlayerState?)"));
		return false;
	}

	// "Initialize From Inventory" is a Blueprint function on plugin content — the spaces
	// are part of the real UFunction name.
	UFunction* InitFn = StashPanel->FindFunction(FName(TEXT("Initialize From Inventory")));
	if (!InitFn)
	{
		UE_LOG(LogStashView, Error, TEXT("StashView: %s has no 'Initialize From Inventory' function"),
			*StashPanel->GetClass()->GetName());
		return false;
	}

	// Fill the parameter block by TYPE, not by name (BP param names carry spaces): the
	// inventory-typed param takes the stash, the filter param (a widget class) stays null —
	// the stock looting menu leaves it unconnected too.
	uint8* Parms = static_cast<uint8*>(FMemory::Malloc(FMath::Max<int32>(1, InitFn->ParmsSize), 16));
	FMemory::Memzero(Parms, InitFn->ParmsSize);
	for (TFieldIterator<FProperty> It(InitFn); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->InitializeValue_InContainer(Parms);
		if (const FObjectProperty* ObjProp = CastField<FObjectProperty>(*It))
		{
			if (Stash->IsA(ObjProp->PropertyClass))
			{
				ObjProp->SetObjectPropertyValue_InContainer(Parms, Stash);
			}
		}
	}
	StashPanel->ProcessEvent(InitFn, Parms);
	for (TFieldIterator<FProperty> It(InitFn); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms);
	}
	FMemory::Free(Parms);

	// gold readout: same component the pane was just initialized from
	if (UTextBlock* GoldText = WidgetTree ? WidgetTree->FindWidget<UTextBlock>(FName(TEXT("Text_Gold"))) : nullptr)
	{
		GoldText->SetText(FText::Format(NSLOCTEXT("NexusStash", "StashGoldFmt", "Gold: {0}"),
			FText::AsNumber(Stash->GetCurrency())));
	}

	UE_LOG(LogStashView, Log, TEXT("StashView: initialized from %s (%d stack(s), %d currency)"),
		*Stash->GetName(), Stash->GetItems().Num(), Stash->GetCurrency());
	return true;
}
