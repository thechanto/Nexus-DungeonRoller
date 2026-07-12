// Fill out your copyright notice in the Description page of Project Settings.

#include "NexusInventoryUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "Components/Widget.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIInputTypes.h"
#include "InputAction.h"

UNexusInventoryUIComponent::UNexusInventoryUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNexusInventoryUIComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("NexusInventoryUI: HUDWidgetClass not set on %s"), *GetNameSafe(GetOwner()));
		return;
	}

	HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(ViewportZOrder);
		UE_LOG(LogTemp, Log, TEXT("NexusInventoryUI: HUD wrapper %s added to viewport (ZOrder %d)"), *GetNameSafe(HUDWidget), ViewportZOrder);
	}

	if (ToggleAction)
	{
		if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			Input->BindAction(ToggleAction, ETriggerEvent::Started, this, &UNexusInventoryUIComponent::ToggleInventoryMenu);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NexusInventoryUI: no EnhancedInputComponent on %s"), *GetNameSafe(PC));
		}
	}
}

UUserWidget* UNexusInventoryUIComponent::ResolveNarrativeHUD() const
{
	if (!HUDWidget)
	{
		return nullptr;
	}

	if (UWidget* Child = HUDWidget->GetWidgetFromName(NarrativeHUDWidgetName))
	{
		if (UUserWidget* ChildUserWidget = Cast<UUserWidget>(Child))
		{
			return ChildUserWidget;
		}
	}

	return HUDWidget;
}

UCommonActivatableWidget* UNexusInventoryUIComponent::OpenMenuByClass(TSubclassOf<UUserWidget> MenuClass)
{
	UUserWidget* NarrativeHUD = ResolveNarrativeHUD();
	if (!NarrativeHUD || !MenuClass)
	{
		return nullptr;
	}

	UFunction* OpenMenuFn = NarrativeHUD->FindFunction(TEXT("OpenMenu"));
	if (!OpenMenuFn)
	{
		UE_LOG(LogTemp, Warning, TEXT("NexusInventoryUI: %s has no OpenMenu function"), *GetNameSafe(NarrativeHUD));
		return nullptr;
	}

	// Layout matches WBP_NarrativeHUD.OpenMenu(ActivatableWidgetClass) -> ReturnValue.
	struct FOpenMenuParams
	{
		UClass* ActivatableWidgetClass;
		UUserWidget* ReturnValue;
	};
	FOpenMenuParams Params{ *MenuClass, nullptr };
	NarrativeHUD->ProcessEvent(OpenMenuFn, &Params);

	UE_LOG(LogTemp, Log, TEXT("NexusInventoryUI: OpenMenu(%s) -> %s"), *GetNameSafe(*MenuClass), *GetNameSafe(Params.ReturnValue));
	return Cast<UCommonActivatableWidget>(Params.ReturnValue);
}

void UNexusInventoryUIComponent::ToggleInventoryMenu()
{
	// If the menu we opened is still up, close it (the CommonUI stack pops it on deactivate).
	if (UCommonActivatableWidget* Open = OpenMenuWidget.Get())
	{
		OpenMenuWidget.Reset();
		if (Open->IsActivated())
		{
			Open->DeactivateWidget();
			return;
		}
	}

	OpenMenuWidget = OpenMenuByClass(InventoryMenuClass);

	// While the menu is the active CommonUI widget it captures game input, so the controller's
	// IA_Inventory binding above never fires to close it. Register the same action on the menu
	// as a CommonUI action binding; the CommonUI action router routes the toggle key to it and
	// closes the menu. Requires CommonInput's EnhancedInputSupport (set in DefaultGame.ini).
	if (UCommonActivatableWidget* Menu = OpenMenuWidget.Get())
	{
		if (ToggleAction)
		{
			FBindUIActionArgs BindArgs(ToggleAction,
				FSimpleDelegate::CreateUObject(this, &UNexusInventoryUIComponent::CloseInventoryMenu));
			BindArgs.bDisplayInActionBar = false;
			Menu->RegisterUIActionBinding(BindArgs);
		}
	}
}

void UNexusInventoryUIComponent::OpenLootMenu()
{
	// The loot source is already set on the run inventory by OpenContainerLoot. Push the
	// looting menu; it reads the run inventory + its LootSource itself and closes on Back
	// (CommonUI) or when looting ends (OnEndLooting). We do not track it as OpenMenuWidget
	// so the I-key inventory toggle stays independent of an open loot menu.
	OpenMenuByClass(LootMenuClass);
}

void UNexusInventoryUIComponent::CloseInventoryMenu()
{
	if (UCommonActivatableWidget* Open = OpenMenuWidget.Get())
	{
		OpenMenuWidget.Reset();
		if (Open->IsActivated())
		{
			Open->DeactivateWidget();
		}
	}
}
