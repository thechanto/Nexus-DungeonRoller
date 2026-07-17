// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NexusMainMenu.generated.h"

class UButton;

/**
 * C++ base for W_MainMenu. Exists because component-bound OnClicked events cannot be
 * authored through scripting: binds Button_Stash by name and opens the stash view.
 * The BP's own hand-made button chains (Play/Quit/SkillTree/Settings) are untouched.
 */
UCLASS()
class NEXUS_API UNexusMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Widget opened by the Stash button (set to W_StashView on the W_MainMenu CDO). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stash")
	TSubclassOf<UUserWidget> StashViewClass;

	/** BlueprintCallable so headless tests can drive the click path. */
	UFUNCTION(BlueprintCallable, Category = "Stash")
	void OpenStashView();

protected:
	virtual void NativeConstruct() override;

	/** Optional so W_MainMenu still compiles before/without the button. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Stash;

private:
	UFUNCTION()
	void HandleStashClicked();
};
