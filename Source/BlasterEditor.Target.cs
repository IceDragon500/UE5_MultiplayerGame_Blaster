// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BlasterEditorTarget : TargetRules
{
	public BlasterEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		
		//BuildEnvironment = TargetBuildEnvironment.Unique;
		
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		//CppStandard = CppStandardVersion.Cpp20;
		
		//WindowsPlatform.bStrictConformanceMode = false;
		
		ExtraModuleNames.Add("Blaster");
	}
}
