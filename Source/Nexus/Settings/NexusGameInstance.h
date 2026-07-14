// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NexusGameInstance.generated.h"

/**
 * Native parent for BP_MyGameInstance (which stays the class named by GameInstanceClass in
 * DefaultEngine.ini -- we reparent the Blueprint rather than replace it).
 *
 * The Blueprint's own Init logic is untouched: UGameInstance::Init calls ReceiveInit() as its
 * first statement, so the BP's Event Init -- which loads the talent data assets -- still runs
 * as part of Super::Init() below.
 *
 * This exists for exactly one reason: the audio device's transient primary volume is not persisted
 * by the engine, so the saved master volume has to be pushed back into it on every launch.
 */
UCLASS()
class NEXUS_API UNexusGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void OnStart() override;
};
