# 📜 Project Wings Implementation History

이 문서는 프로젝트의 현재 진행 상태와 구현 완료된 히스토리를 기록합니다. 세션 시작 시 AI는 이 내용을 바탕으로 문맥을 파악합니다.

---

## 🎯 현재 목표 (Current Goal)
- [x] **아이템 및 드롭 시스템 구현:** 적 처치 시 재화 및 회복 달걀 드롭 로직 구현.
- [x] **낮과 밤 사이클 시스템 구현:** 시간의 흐름에 따른 조명 변화 및 게임 페이즈 전환.
- [ ] **궁극기(Q 스킬) 시스템 구현:** 적 처치 시 게이지 상승 및 TPS 시점 전환 난사 로직.

---

## 🚧 진행 중 / 다음 작업 (To-Do)
1. `ADuckCharacter`에 궁극기 게이지 및 Q 키 입력 바인딩.
2. 궁극기 발동 시 카메라 블렌드 (`SetViewTargetWithBlend`) 및 TPS 시점 로직.
3. 적 스폰 시스템: 밤 페이즈에만 맵 외곽에서 적들이 생성되도록 구현.

---

## ✅ 완료된 기록 (History Log)

### 2026-05-14 (현재)
- **낮과 밤 사이클 및 환경 시스템:**
  - `ADuckGameMode`: 24시간 시계 및 게임 페이즈(Night, Morning, Day) 시스템 구현.
  - **정밀한 태양 궤적:** 쿼터니언 Slerp 보간을 이용해 6시(일출), 12시(정오), 18시(일몰) 수치를 정확히 통과하며 비스듬하게 회전하는 물리 기반 태양 궤도 구현.
  - **조명 최적화:** 6시 일출 시점 조명 밝기 동기화 및 에디터 수치 튐 현상 방지.
- **적 AI 행동 고도화 (Phase Interaction):**
  - **Morning Phase 대응:** 아침 6시가 되면 모든 적이 4초간 정지 후 플레이어 반대 방향으로 도망치는 로직 구현.
  - **시선 처리 개선:** 추격 시에는 플레이어를, 도망 시에는 도망 방향을 자연스럽게 바라보도록 `RInterpTo` 기반 회전 로직 적용.
- **아이템 시스템 고도화 및 클래스 분리:**
  - `AGoldItem`, `AAmmoItem` 구현 및 플레이어 스탯(Gold, Ammo Gauge) 연동.
  - `ADropItemBase` 물리 보강: 소환 시 강제로 `Simulate Physics`를 활성화하여 튕겨나가는 연출 보장.
- **빌드 오류 수정:**
  - `DuckCharacter`의 `Gold` 변수 접근 제한자 및 메타 태그(`AllowPrivateAccess`) 오류 수정.
  - `GoldItem`에서 `private` 멤버 직접 접근 대신 `Getter`를 사용하도록 수정.
- **데미지 및 UI 시스템 구현:**
  - `EnemyBase.cpp` 및 `DuckCharacter.cpp`에서 `Public/` 접두사가 포함된 잘못된 헤더 포함 경로 수정 (`C1083` 오류 해결).
- **데미지 및 UI 시스템 구현:**
  - `AEnemyBase` 체력 시스템: `TakeDamage` 오버라이드 및 `CurrentHealth/MaxHealth` 관리.
  - `Die()` 로직 구현: 체력 0 이하 시 액터 파괴 및 아이템 드롭 준비.
  - `HealthBarWidget`: 머리 위 `WidgetComponent` 추가 및 카메라를 향하는 Screen Space UI 설정.
  - `AEggProjectile` 데미지 연동: 명중 시 물리적 넉백과 함께 실제 데미지(20.0) 전달.
- **물리 기반 적 AI 및 상호작용 고도화:**
  - `APawn` 기반 물리 폰 리팩토링: `ACharacter`에서 전환하여 `Simulate Physics` 루트 이동 구현.
  - `BoxComponent` 도입: 캡슐 대신 박스를 루트로 사용하여 단단한 물리 상호작용 및 겹침 방지.
  - **연속 충돌 감지(CCD):** 발사체와 몬스터에 CCD를 활성화하여 고속 관통 현상 해결.
  - **지면 감지(Grounded Check):** LineTrace를 통해 바닥에 닿았을 때만 점프하도록 개선(공중 부유 해결).
  - **물리 안정화:** 넉백 시 기존 속도 상쇄 및 최대 속도 제한(Clamping) 적용.
  - **독립적 스케일 설정:** `SetAbsolute(Scale=true)`를 통해 메시 크기 변화 없이 충돌 영역만 조절 가능하도록 구현.
- **적 AI 기초 구현:**
  - `AEnemyBase` 클래스 생성 및 AI 추격 로직 구현.
  - `EggThiefDuck.Build.cs`: `AIModule`, `NavigationSystem`, `UMG` 모듈 의존성 추가.
- **전투 시스템 기초 구현:**
  - `AEggProjectile` 클래스 구현: 발사체 이동 및 충돌 로직.
  - `UDuckCombatComponent` 클래스 구현: 달걀 게이지(탄창), 자동 회복, 과열(Overheat) 로직.
  - `ADuckCharacter` 통합: 전투 컴포넌트 부착 및 마우스 좌클릭(FireAction) 입력 바인딩 완료.
- **기본 캐릭터 시스템 구현:**
  - `ADuckCharacter` 클래스 생성.
  - 탑뷰 카메라 시스템 (SpringArm + Camera) 설정.
  - Enhanced Input을 이용한 8방향 이동 구현.
  - 마우스 커서 위치를 추적하는 실시간 회전(LookAt) 로직 구현.
- **입력 에셋 설정:**
  - `IA_Move`, `IMC_Duck` 생성 및 적용.
- **프로젝트 초기화:**
  - `GEMINI.md`, `GAME_DESIGN.md`, `history.md` 작성 및 동기화.
