# 📜 Project Wings Implementation History

이 문서는 프로젝트의 현재 진행 상태와 구현 완료된 히스토리를 기록합니다. 세션 시작 시 AI는 이 내용을 바탕으로 문맥을 파악합니다.

---

## 🎯 현재 목표 (Current Goal)
- [ ] **적 AI 기초 구현:** `AEnemyBase` 클래스 생성 및 애니메이션 에셋 없는 '통통 튀는(Hopping)' 움직임 로직 구현.

---

## 🚧 진행 중 / 다음 작업 (To-Do)
1. `AEnemyBase` 클래스 생성 (기본 적 부모 클래스).
2. `Squash & Stretch` 기법을 이용한 코드로 구현하는 점프 이동 로직.
3. 플레이어 추격 (Simple Move to Actor) 로직 연동.
4. 에디터 작업 필요: `IA_Fire` 생성 및 `BP_DuckCharacter`에 사격 관련 에셋 할당.

---

## ✅ 완료된 기록 (History Log)

### 2026-05-13 (현재)
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