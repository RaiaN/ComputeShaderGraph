namespace UnrealBuildTool.Rules
{
	public class ComputeShader : ModuleRules
	{
		public ComputeShader(ReadOnlyTargetRules Target) : base(Target)
        {
            bEnforceIWYU = true;

            bUseUnity = false;

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "RenderCore",
					"Renderer",
					"RHI",
                    "Projects"
				}
			);
		}
	}
}