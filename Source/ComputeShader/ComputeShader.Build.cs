namespace UnrealBuildTool.Rules
{
	public class ComputeShader : ModuleRules
	{
		public ComputeShader(ReadOnlyTargetRules Target) : base(Target)
        {
            bEnforceIWYU = true;

            MinFilesUsingPrecompiledHeaderOverride = 1;
            bFasterWithoutUnity = true;

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "RenderCore",
                    "RHI",
                    "Projects"
				}
			);
		}
	}
}