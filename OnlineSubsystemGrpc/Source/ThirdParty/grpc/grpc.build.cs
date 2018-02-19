using UnrealBuildTool;
using System.IO;

public class grpc : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    public grpc(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string grpcLibPath = Path.Combine(ModulePath, "lib");
        string grpcDepLibPath = Path.Combine(ModulePath, "libs");
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string win64LibPath = Path.Combine(grpcLibPath, "Win64");
            string win64DepLibPath = Path.Combine(grpcDepLibPath, "Win64");
            PublicAdditionalLibraries.Add(Path.Combine(win64LibPath, "grpc_unsecure.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(win64LibPath, "gpr.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(win64LibPath, "grpc++_unsecure.lib"));

            PublicAdditionalLibraries.Add(Path.Combine(win64DepLibPath, "zlibstatic.lib"));
            PublicIncludePaths.Add(Path.Combine(ModulePath, "include"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
        }
    }
}