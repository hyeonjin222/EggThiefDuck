# 📜 Project Wings Implementation History

이 문서는 프로젝트의 현재 진행 상태와 구현 완료된 히스토리를 기록합니다. 세션 시작 시 AI는 이 내용을 바탕으로 문맥을 파악합니다.

---

## 🎯 현재 목표 (Current Goal)
- [ ] **아이템 및 드롭 시스템 구현:** 적 처치 시 재화 및 회복 달걀 드롭 로직 구현.

---

## 🚧 진행 중 / 다음 작업 (To-Do)
1. `ADropItemBase` 클래스 및 상속 클래스(Gold, Ammo) 생성.
2. `AEnemyBase`에 사망 로직 및 아이템 스폰 확률 연동.
3. 플레이어 아이템 획득 범위(Overlap) 및 효과 적용.

---

## ✅ 완료된 기록 (History Log)

### 2026-05-13 (현재)
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
