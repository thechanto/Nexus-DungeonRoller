// Fill out your copyright notice in the Description page of Project Settings.

#include "NexusMainMenu.h"
#include "Components/Button.h"

DEFINE_LOG_CATEGORY_STATIC(LogStashView, Log, All);

void UNexusMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Stash)
	{
		Button_Stash->OnClicked.AddUniqueDynamic(this, &UNexusMainMenu::HandleStashClicked);
	}
	else
	{
		UE_LOG(LogStashView, Warning, TEXT("MainMenu: no Button_Stash child bound"));
	}
}

void UNexusMainMenu::HandleStashClicked()
{
	OpenStashView();
}

void UNexusMainMenu::OpenStashView()
{
	if (!StashViewClass)
	{
		UE_LOG(LogStashView, Warning, TEXT("MainMenu: StashViewClass is unset on the CDO"));
		return;
	}

	if (UUserWidget* View = CreateWidget<UUserWidget>(GetOwningPlayer(), StashViewClass))
	{
		View->AddToViewport(10);
		UE_LOG(LogStashView, Log, TEXT("MainMenu: opened stash view %s"), *View->GetName());
	}
}
