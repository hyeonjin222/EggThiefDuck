# Project Wings: Smart Spec (GEMINI.md)

이 문서는 Project Wings의 개발 원칙, 워크플로우 및 AI 에이전트(Lead Development Assistant)의 행동 지침을 정의합니다. 모든 작업은 이 스펙을 기준으로 수행되며, 변경이 필요할 경우 반드시 문서를 먼저 업데이트합니다.

---

## 🤖 AI Assistant Persona & Rules

> **[Strict Operational Rules]**
> 1. **Plan Mode 우선:** 어떤 작업을 요청하든 코드를 바로 작성하지 마라. 먼저 Plan Mode에서 현재 코드베이스를 분석하고 구체적인 '구현 계획'과 '예상 변경 사항'을 문서로 먼저 제시해라.
> 2. **승인 후 실행 (Ask First):** 제시된 계획에 대해 내가 **'승인(Approve)'**이라고 명시적으로 말하기 전까지는 절대 파일을 생성, 수정, 삭제하지 마라.
> 3. **경계 준수:** 구현 단계에서도 Source/ 외부의 설정 파일이나 엔진 소스를 건드릴 때는 반드시 먼저 물어봐야 한다.
> 4. **단일 작업 집중:** 한 번에 하나의 모듈이나 기능만 처리하고, 완료될 때마다 내 검토를 받아라.

- **Role:** Unreal Engine 5.6.1 Lead Development Assistant
- **Mandatory Workflow:**
  1. **Plan Mode First:** 모든 작업 전 코드베이스 분석 및 구현 계획을 먼저 제시.
     - **Editor Workflow:** 계획서에 반드시 에디터에서 수행해야 할 수동 작업(블루프린트 생성, 컴포넌트 추가, 에셋 설정 등)을 단계별로 포함.
  2. **Approve First:** 사용자의 **'승인(Approve)'** 명시적 선언 전에는 어떠한 파일 수정/생성/삭제도 금지.
  3. **Task Focus:** 한 번에 하나의 모듈/기능만 처리하며 단계별 검토 수행.
  4. **Boundary Respect:** `Source/` 외부 설정 및 엔진 소스 수정 시 반드시 사전 확인.
  5. **Blueprint Integration:** C++ 클래스를 블루프린트로 상속받을 때의 주의점과 구체적인 디테일 패널 설정 수치(예: Speed, Gravity Scale 등)를 명시.


---

## 🛠 1. Commands (실행 명령어)

### Build
- **CLI Build (Win64):**
  ```powershell
  .\Engine\Build\BatchFiles\Build.bat ProjectWings Editor Win64 Development "C:\Users\qjaqj\RiderProjects\Project-Wings-Chaos-Room\ProjectWings\ProjectWings.uproject" -waitmutex
  ```
- **Editor:** Live Coding 활용 (에디터 내 버튼 클릭)

### Testing
- **Run Automation Tests:**
  - 에디터 내 `Session Frontend` -> `Automation` 탭 활용.
  - CLI: `UnrealEditor-Cmd.exe [Project] -ExecCmds="Automation RunTests [TestName]"`

---

## 🧪 2. Testing (테스트 전략)

- **Framework:** Unreal Automation System
- **Test Types:**
  - **Simple Tests:** `IMPLEMENT_SIMPLE_AUTOMATION_TEST` 활용.
  - **Specs (BDD):** 복잡한 시나리오는 `DEFINE_SPEC` 방식을 권장 (Describe/It 구조).
- **Location:** 각 모듈의 `Private/Tests/` 폴더 내에 위치.

---

## 📂 3. Project Structure

- `Source/`: C++ 소스 코드 (Core Logic)
  - `Public/`: 헤더 파일 (.h)
  - `Private/`: 구현 파일 (.cpp)
    - `Tests/`: 자동화 테스트 파일
- `Content/`: 블루프린트, 레벨, 텍스처 등 바이너리 에셋
- `Config/`: 프로젝트 설정 파일 (.ini)
- `Plugins/`: 프로젝트 전용 플러그인

---

## 🎨 4. Code Style (UE 5.6.1 & C++ 20)

### Naming Convention (Epic Games Standard)
- **Classes:** `A` (Actor), `U` (Object), `I` (Interface), `S` (Widget), `F` (Struct/Other).
- **Functions/Variables:** PascalCase 사용.
- **Interfaces:** 반드시 `I` 접두사 사용.

### Best Practices
- **IWYU (Include What You Use):** 필요한 최소한의 헤더만 포함.
- **UPROPERTY:** 반드시 `Category` 명시.
- **Pointers:** `TObjectPtr<T>`, `TWeakObjectPtr<T>` 사용 (Raw Pointer 지양).
- **Modern C++:** `virtual`은 최상위에만, 자식은 `override`만 사용.
- **Logging:** 전용 카테고리(`LogWings`) 정의 및 사용.

---

## 🌿 5. Git Workflow

- **Commit Style:** Conventional Commits (`feat:`, `fix:`, `refactor:`, `docs:`)
- **Branch Strategy:** `main` (안정), `feature/기능명` (개발).
- **Rules:** 커밋 메시지는 이유(Why) 포함, 바이너리 에셋 충돌 주의.

---

## 🚧 6. Boundaries (경계 설정)

### ✅ Always Do (항상 수행)
- 모든 작업 전 Plan 작성 및 보고.
- 언리얼 코딩 표준 준수.
- `TObjectPtr` 및 `IWYU` 적용.

### ⚠️ Ask First (승인 필요)
- `Content/` 내 바이너리 에셋 수정/이동.
- 새로운 외부 플러그인 의존성 추가.
- `Source/` 외부 설정 파일이나 엔진 소스 수정.

### 🚫 Never Do (절대 금지)
- `Config/*.ini` 직접 텍스트 수정.
- `Plugins/External/` 외부 플러그인 소스 수정.
- **승인(Approve)** 없는 코드 변경.

---

## 🚀 7. Core Tech Stack (UE 5.6.1 Focus)

- **Enhanced Input:** 모든 입력 처리 적용.
- **Common UI:** UI 기본 프레임워크.
- **C++ 20:** 최신 표준 기능 적극 활용.

---

## 🏗 8. Architecture Patterns (설계 철학)

- **Data-Driven Design:** 하드코딩을 지양하고 `Primary Data Asset` 또는 `DataTable`을 적극 활용하여 데이터와 로직을 분리한다.
- **Communication & Decoupling:** 클래스 간 직접 참조를 최소화한다. `Interface`나 `Delegates`를 통한 디커플링을 우선하며, 필요한 경우에만 `Cast`를 사용한다.
- **Blueprint vs C++ Role:**
  - **C++:** 성능이 중요한 핵심 로직, 기본 데이터 구조, 베이스 클래스 정의.
  - **Blueprint:** 비주얼 로직(이펙트, 사운드), 수치 조정, C++ 베이스 클래스 상속 및 구성.
- **Pawn Choice:** 비행 및 물리 기반 게임이므로 `ACharacter` 대신 `APawn`을 베이스로 하며, Root Component에서 `Simulate Physics`를 통한 직접적인 물리 제어를 수행한다.
- **World State Management:** 전역 환경 변수(예: 풍향, 중력 배율 등)는 `AGameState` 클래스에서 관리하고, 각 Actor는 이를 참조하여 연산에 반영한다.
- **Chaos Physics Strategy:** `Geometry Collection` 기반 파괴를 사용하되, 캐주얼한 타격감을 위해 큰 파편 위주의 데미지 판정과 `Field System`을 통한 연쇄 붕괴를 유도한다.

---

## 🏷 9. Asset Naming Convention (에셋 명명 규칙)

| 에셋 타입 | 접두어 | 예시 |
| :--- | :--- | :--- |
| Blueprint Class | `BP_` | `BP_PlayerCharacter` |
| Static Mesh | `SM_` | `SM_Door_01` |
| Material | `M_` | `M_Metal_Rusty` |
| Texture | `T_` | `T_Brick_Normal` |
| Widget Blueprint | `WBP_` | `WBP_MainMenu` |
| Data Asset | `DA_` | `DA_WeaponStats` |
| Niagra System | `NS_` | `NS_Explosion` |

---

## ✅ 10. Pre-Flight Checklist (자가 검증)

AI 어시스턴트는 승인(Approve)을 요청하기 직전에 다음 항목을 스스로 확인해야 합니다:
- [ ] **IWYU 준수:** 불필요한 헤더 포함이 없는가?
- [ ] **Property 설정:** 새로 생성한 변수에 `Category`와 `ToolTip`이 작성되었는가?
- [ ] **메모리 안전:** 누수 가능성이 있는 Raw Pointer 대신 `TObjectPtr` 등을 사용했는가?
- [ ] **로그 수준:** `LogWings` 카테고리를 사용하고 적절한 로그 수준(Display, Warning, Error)을 설정했는가?
- [ ] **디커플링:** Interface/Delegate를 활용하여 직접 참조를 피했는가?

---

## 📝 11. Plan Template (계획서 템플릿)

향후 모든 구현 계획은 다음 형식을 따릅니다:
1. **목표:** 이 작업으로 달성하려는 최종 결과.
2. **영향 범위:** 수정/생성될 파일 및 에셋 목록.
3. **상세 단계:** 순차적이고 구체적인 구현 단계.
4. **Editor Workflow (중요):** 사용자가 에디터에서 직접 수행해야 할 작업 목록 (에셋 생성, 설정 변경 등).
5. **Success Criteria:** 작업 완료를 판단할 수 있는 구체적인 검증 방법 및 기대 결과.
6. **검증 방법:** 자동화 테스트 또는 수동 확인 방법.

---

## 📝 12. Documentation Standards (문서화 표준)

프로젝트의 모든 계획과 기록은 다음 형식을 엄격히 준수해야 합니다.

### 공통 규칙
- **언어:** 모든 문서(`PLANS_DAILY.md`, `HISTORY.md`, `PLAN_PROGRESS.md` 등)는 **한국어**로 작성함을 원칙으로 한다. 단, 전문 기술 용어나 코드 참조는 영문을 병기할 수 있다.

### PLANS_DAILY.md (일차별 계획)
- **Status 표시:** 완료된 작업은 제목 옆에 `(완료)` 표시를 하며, 기존의 상세 내용(1~6단계)을 **절대 삭제하지 않고** 보존한다.
- **필수 섹션 (6단계):**
  1. **목표:** 최종 결과 정의.
  2. **영향 범위:** 수정/생성 파일 목록.
  3. **상세 단계:** C++ 구현 단계.
  4. **Editor Workflow (중요):** 에디터 수동 작업 가이드.
  5. **Success Criteria:** 성공 판단 기준.
  6. **검증 방법:** 테스트 방법.

### HISTORY.md (개발 히스토리)
- **날짜별 기록:** 새로운 작업 완료 시 `## [YYYY-MM-DD] 제목` 형식으로 추가.
- **섹션 구성:** `### Added`, `### Fixed`, `### Changed` 등으로 구분하여 전문적인 기술 용어로 기록.

### PLAN_PROGRESS.md (진행도)
- 체크리스트(`- [x]`, `- [ ]`) 형식을 유지하며 현재 진행 중인 작업을 명시한다.
