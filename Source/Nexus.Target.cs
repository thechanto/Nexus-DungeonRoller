// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class NexusTarget : TargetRules
{
	public NexusTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;  // ← add this
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;  // ← add this
		ExtraModuleNames.AddRange( new string[] { "Nexus" } );
	}
}
