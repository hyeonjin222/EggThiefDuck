# 🤖 Lead Development Assistant Spec (GEMINI.md)

이 문서는 프로젝트의 개발 원칙 및 AI 어시스턴트의 행동 지침을 정의합니다. 언리얼엔진 에디터의 언어는 한글이기 때문에 한글 기준으로 설명.

---

## 🛠 AI Assistant Rules

> **[운영 원칙]**
> 1. **컨텍스트 최우선 준수:** 세션 시작 시 제공된 3개의 문서 `GAME_DESIGN.md`(기획), `GEMINI.md`(규칙), `history.md`(현재 상태)의 내용을 완벽히 숙지하고, 이를 세션 내내 모든 답변과 코드 작성의 절대적인 기준으로 삼으십시오.
> 2. **히스토리 기록:** 기능 구현이 완료되면 반드시 변경 사항을 요약하여 `history.md`에 기록하십시오.

---

## 🎨 1. Code Style (UE 5.6.1 & C++ 20)

- **Naming:** Classes(`A`, `U`, `I`, `F`), Variables(`PascalCase`, Bool은 `b` 접두어).
- **Best Practices:** IWYU 준수, `TObjectPtr` 사용, `UPROPERTY(Category)` 필수.
- **Modern C++:** `override` 명시, 언리얼 표준 컨테이너 사용.

---

## ✅ 2. Pre-Flight Checklist (자가 검증)

승인 요청 전 AI 스스로 확인:
- [ ] IWYU 및 언리얼 명명 규칙 준수 여부
- [ ] `TObjectPtr` 및 메모리 안전성 확인
- [ ] 불필요한 `Tick` 사용 여부 및 이벤트 기반 설계 확인

---

## 🏗 3. Implementation Plan Template (계획서 형식)

1. **목표:** 구현 기능 정의
2. **수정 범위:** 영향 받는 파일 목록
3. **상세 단계:** C++ 로직 및 에디터 작업 단계
4. **검증 방법:** 정상 작동 확인 방법