using UnrealBuildTool;
using System.Collections.Generic;

public class UEHomework1EditorTarget : TargetRules
{
	public UEHomework1EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		ExtraModuleNames.AddRange( new string[] { "UEHomework1" } );
	}
}
