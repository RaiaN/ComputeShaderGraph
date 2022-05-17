// Copyright Peter Leontev

namespace UnrealBuildTool.Rules
{
	public class ComputeShaderGraphEditor : ModuleRules
	{
		public ComputeShaderGraphEditor(ReadOnlyTargetRules Target) : base(Target)
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