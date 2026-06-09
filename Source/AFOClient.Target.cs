// AFOClient.Target.cs
using UnrealBuildTool;
using System.Collections.Generic;

public class AFOClientTarget : TargetRules
{
    public AFOClientTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Client; // 이 타겟이 클라이언트임을 명시

        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.AddRange(new string[] { "AFO" });
    }
}