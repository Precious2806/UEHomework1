using UnrealBuildTool;

public class UEHomework1 : ModuleRules
{
	public UEHomework1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "EnhancedInput", "AIModule", "NavigationSystem", "GameplayTasks" });
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
	}
}
