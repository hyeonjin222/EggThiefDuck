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
- **적 AI 및 물리 상호작용 구현:**
  - `AEnemyBase` 클래스 구현: 코드로 구현한 `Squash & Stretch` 방식의 통통 튀는(Hopping) 움직임.
  - AI 추격 로직: `SimpleMoveToActor`를 이용한 주기적 플레이어 추적.
  - 물리 상호작용 강화: 몬스터-플레이어-몬스터 간 충돌 및 밀기 기능, 달걀 피격 시 넉백(Knockback) 시스템 적용.
  - `AEggProjectile` 업데이트: 충돌 시 물리적 임펄스 전달 및 도탄/중력 설정 추가.
  - `EggThiefDuck.Build.cs`: AI 및 Navigation 모듈 의존성 추가.
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