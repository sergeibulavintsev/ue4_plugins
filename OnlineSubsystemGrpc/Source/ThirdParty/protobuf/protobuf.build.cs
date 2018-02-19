using UnrealBuildTool;
using System.IO;

public class protobuf : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    public protobuf(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string libPath = Path.Combine(ModulePath, "lib");
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string win64LibPath = Path.Combine(libPath, "Win64");
            PublicAdditionalLibraries.Add(Path.Combine(win64LibPath, "libprotobuf.lib"));
            PublicIncludePaths.Add(Path.Combine(ModulePath, "include"));
            //Definitions.AddRange(
            //new string[]
            //{
            //                "WIN64",
            //                "_WINDOWS",
            //                "NDEBUG",
            //                "GOOGLE_PROTOBUF_CMAKE_BUILD",
            //});
        }

        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
        }
    }
}