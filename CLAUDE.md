# CLAUDE.md

This file provides guidance to Claude Code when working with this repository.

---

## Project Overview

- **프로젝트명:** AFO_Refactor
- **원본:** 팀 프로젝트 AFO (2v2 멀티플레이 액션, Dedicated Server)
- **목적:** 팀프로젝트 AFO의 전투/서버 구조를 GAS 기반으로 리팩토링 + 알려진 버그 수정
- **엔진:** Unreal Engine 5.5
- **플랫폼:** PC (Windows) — 모바일 최적화 원칙 적용
- **언어:** C++ (Blueprint 최소화)
- **목표 FPS:** 60fps 안정 (모바일 기준 30fps 원칙 적용)
- **목표 드로우콜:** 150 이하
- **개발 목적:** 넷마블 클라이언트 프로그래머 취업 포트폴리오

---

## Build & Development

**프로젝트 파일 재생성 (h/cpp 추가/삭제 후 필요):**
```
"C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\GenerateProjectFiles.bat" AFO.uproject -game
```

**커맨드라인 빌드:**
```
"C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" AFO Win64 Development "C:\[프로젝트경로]\AFO.uproject"
```

**Dedicated Server 빌드:**
```
"C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" AFOServer Win64 Development "C:\[프로젝트경로]\AFO.uproject"
```

**일반 워크플로우:**
Visual Studio 2022에서 `AFO.sln` 열기 → 시작 프로젝트를 `AFO`로 설정 → `Ctrl+B` 빌드

---

## Architecture

### Module Structure
- 단일 런타임 모듈: `AFO` (`Source/AFO/`)
- 모듈 의존성: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `UMG`, `Slate`, `SlateCore`, `Niagara`
- GAS 추가 시: `GameplayAbilities`, `GameplayTags`, `GameplayTasks` 추가 필요
- PCH 모드: `UseExplicitOrSharedPCHs`
- 타겟: `AFO` (Game), `AFOServer` (Server), `AFOEditor` (Editor), `AFOClient` (Client)

### 폴더 구조
```
Source/AFO/
├── Animation/      ← AnimInstance, AnimNotify
├── Character/      ← AFPlayerCharacter (베이스), 캐릭터 파생 클래스
├── Components/     ← AFAttributeComponent (→ GAS로 교체 예정), AFStatusEffectComponent
├── Controller/     ← AFLobbyPlayerController
├── Game/           ← GameMode, GameState, GameInstance, GameSession
├── Gimmick/        ← 버프 아이템, 아이템 스포너
├── Player/         ← AFPlayerController, AFPlayerState, AFTitlePlayerController
└── UI/             ← 인게임 HUD, ESC 메뉴, 킬 로그, 리스폰 위젯 등
```

### Game Flow
```
Title맵 (AFTitlePlayerController)
  → TeamSelect맵 (AFTeamSelectGameMode / AFLobbyPlayerController)
  → CharacterSelect맵 (AFCharacterSelectGameMode / AFLobbyPlayerController)
  → BattleZone (AFGameMode / AFPlayerController)
```

### Network Architecture
- Dedicated Server 기반
- 모든 게임 로직은 서버에서 처리 (`HasAuthority()` 필수 체크)
- 클라이언트는 RPC 또는 Replicated 변수로만 결과를 수신
- PlayerState를 통해 HP/Mana/KillCount/TeamInfo 복제

---

## 코드 작성 원칙

### 서버 권한 처리 (최우선)
```cpp
// 모든 게임 로직 함수 시작부에 반드시 체크
if (!HasAuthority()) return;
```

### RPC 패턴
```
클라이언트 입력
  → Server RPC (서버에서 검증 + 처리)
  → Multicast RPC (모든 클라이언트에 시각 효과 전파)
```

### 의존성 제거 원칙
- Cast 남발 금지 → Interface 사용
- 하드코딩 금지 → DataAsset / DataTable 분리
- Tick 남용 금지 → Timer / Event / Delegate 기반으로 교체

### 모바일 최적화 원칙 (수치 기준)
- 드로우콜: 150 이하 (stat scenerendering으로 측정)
- Tick 함수: 반드시 필요한 경우만 사용
- Cast: 매 프레임 Cast 금지, 캐시 또는 Interface 사용
- Object Pool: 빈번한 Spawn/Destroy 대신 Pool 활용
- 라이팅: Static/Stationary만 사용

---

## 알려진 버그 및 리팩토링 대상

### 🔴 Critical (즉시 수정)
1. **HandleSkillHitCheck 세미콜론 버그**
   - 위치: `AFPlayerCharacter.cpp`
   - 문제: `if (bHit) return;` → 세미콜론으로 인해 데미지 블록이 항상 실행되지 않음
   - 수정: `if (!bHit) return;` 또는 `if (bHit) { ... }` 구조로 변경

### 🟠 High (구조 개선)
2. **AnimInstance Cast 체인**
   - 위치: `AFAnimInstance.cpp` `NativeUpdateAnimation()`
   - 문제: 매 프레임 DarkKnight/Mage/Aurora Cast 3회 실행
   - 수정: `IBFSprintInterface` 또는 `bIsSprinting` 변수를 베이스 클래스로 이동

3. **패키징 후 멀티플레이 크래시**
   - 문제: 클라이언트 측 코드에서 서버 권한 작업(데미지 등) 직접 처리
   - 수정: HasAuthority() 체크 + RPC 구조 전면 정리

### 🟡 Medium (품질 개선)
4. **UI EnsureUI 반복 타이머**
   - 위치: `AFLobbyPlayerController.cpp`
   - 문제: 0.2초마다 `SetupUIForCurrentMap()` 반복 호출
   - 수정: 이벤트/델리게이트 기반으로 교체

5. **맵 이름 하드코딩**
   - 위치: `AFLobbyPlayerController.cpp`
   - 문제: `if (MapName == TEXT("AFOTeamSelect"))` 하드코딩
   - 수정: DataAsset 또는 GameInstance 변수로 분리

6. **팀 배정 인덱스 취약성**
   - 위치: `AFGameMode.cpp` `PostLogin()`
   - 문제: `PlayerTeams.Num() % 2` 기준 팀 배정 → 재접속 시 꼬임 가능
   - 수정: 팀별 카운트 직접 관리로 변경

---

## GAS 도입 계획

### 교체 대상
- `UAFAttributeComponent` → `UAbilitySystemComponent` + `UAFAttributeSet`
- `UAFPlayerState`의 HP/Mana 직접 관리 → `GameplayEffect`로 교체
- 스킬 (SkillE, SkillQ) → `UGameplayAbility`로 교체

### 추가할 모듈 (AFO.Build.cs)
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "GameplayAbilities",
    "GameplayTags",
    "GameplayTasks"
});
```

### GAS 기본 흐름
```
UAbilitySystemComponent (ASC) → PlayerState에 부착
UAFAttributeSet → HP, Mana, MaxHP, MaxMana 정의
UGameplayEffect → 데미지, 힐, 마나 소모 처리
UGameplayAbility → 스킬 E, 스킬 Q, 기본 공격
```

---

## MCP 서버

| 서버 | 역할 |
|------|------|
| context7 | UE5 공식 문서 실시간 참조. GAS API 등 버전별 최신 정보 주입 |
| sequential-thinking | 복잡한 구조 설계 결정 시 단계별 분해에 활용 |
| unreal-mcpython | UE5 에디터 직접 제어. 액터 배치, 에셋 조작 자동화 |

---

## Git 규칙

**.gitignore 필수 항목 (에셋 제외, 소스만 관리):**
```
Content/
Binaries/
Intermediate/
Saved/
DerivedDataCache/
*.VC.db
*.opensdf
*.sdf
*.sln
*.suo
```

**LFS 불필요** — Content 폴더를 올리지 않으므로 LFS 없이 운영

**커밋 규칙:**
- 기능/수정 단위로 커밋
- `feat:` / `fix:` / `refactor:` Conventional Commits 방식
- 커밋 메시지는 한국어로
- Co-Authored-By 태그 유지

---

## 개발 로드맵

| Phase | 작업 | 블로그 |
|-------|------|--------|
| #0 | 프로젝트 셋업 + 코드 분석 + 버그 목록화 | ✅ |
| #1 | HandleSkillHitCheck 버그 수정 | 예정 |
| #2 | Cast 체인 → Interface 교체 | 예정 |
| #3 | GAS 도입 (ASC + AttributeSet 셋업) | 예정 |
| #4 | HP/Mana → GameplayEffect 교체 | 예정 |
| #5 | 스킬 → GameplayAbility 교체 | 예정 |
| #6 | UI 타이머 → 이벤트 기반 교체 | 예정 |
| #7 | 드로우콜 측정 + 최적화 Before/After | 예정 |
| #8 | 패키징 + 멀티플레이 크래시 수정 | 예정 |

---

## 면접 어필 포인트 (작업 시 항상 염두)

1. **문제 발견 → 원인 분석 → 구조적 해결** 흐름을 코드와 블로그에 명시
2. Before/After 수치 반드시 기록 (`stat fps`, `stat unit`, `stat scenerendering`)
3. "왜 이 구조를 선택했는가"를 주석 또는 블로그에 설명
4. 넷마블 우대사항 키워드 의식적으로 연결:
   - GAS → 전투 시스템 설계 능력
   - HasAuthority / RPC → 네트워크/리플리케이션
   - Tick 제거 / Object Pool → 모바일 최적화
   - stat scenerendering → 프로파일링 경험
