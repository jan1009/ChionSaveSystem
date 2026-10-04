using UnrealBuildTool;

public class ChionSaveSystemEditor : ModuleRules
{
    public ChionSaveSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "Projects",
                "DeveloperSettings",
                "Settings",
                "HTTP",
                "Json",
                "JsonUtilities",
                "Slate",
                "SlateCore"
            }
        );
    }
}