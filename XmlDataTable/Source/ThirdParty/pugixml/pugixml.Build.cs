using UnrealBuildTool;
using System.IO;

public class pugixml : ModuleRules
{
    public pugixml(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string LibPath = Path.Combine(ModuleDirectory, "lib");
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string win64LibPath = Path.Combine(LibPath, "Win64");
            PublicAdditionalLibraries.Add(Path.Combine(win64LibPath, "pugixml.lib"));
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
        }
    }
}