using UnrealBuildTool;
using System.Collections.Generic;

public class UEHomework1Target : TargetRules
{
	public UEHomework1Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		ExtraModuleNames.AddRange( new string[] { "UEHomework1" } );
	}
}
