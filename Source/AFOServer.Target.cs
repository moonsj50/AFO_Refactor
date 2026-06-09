// AFOServer.Target.cs
using UnrealBuildTool;
using System.Collections.Generic;

public class AFOServerTarget : TargetRules
{
    public AFOServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server; // 이 타겟이 서버임을 명시

        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.AddRange(new string[] { "AFO" });
    }
}