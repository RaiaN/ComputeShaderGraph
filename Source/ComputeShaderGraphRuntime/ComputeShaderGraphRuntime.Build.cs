// Copyright Peter Leontev

namespace UnrealBuildTool.Rules
{
	public class ComputeShaderGraphRuntime : ModuleRules
	{
		public ComputeShaderGraphRuntime(ReadOnlyTargetRules Target) : base(Target)
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