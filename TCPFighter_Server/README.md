# TCPFighter Server — 소스 코드

select 모델 기반 실시간 2D 대전 게임 서버의 핵심 구현입니다. (1인 단독 구현)

| 파일 | 내용 |
|---|---|
| `main.cpp` | 메인 루프(Network/Update 분리) · select 멀티플렉싱 · 세션 관리(지연 삭제) · 서버 측 판정(이동·공격) · 패킷 프레이밍/생성 |
| `RingBuffer.h` / `ringbuffer.cpp` | 직접 구현한 링버퍼 — `DirectEnqueueSize`로 `recv()` 직접 적재(무복사 수신 경로) |

> **빌드 안내** — 패킷 프로토콜 정의 헤더(`_PacketDefine.h`)는 교육기관 제공 자료라 저작권을 존중해 저장소에서 제외했습니다. 해당 헤더의 `dfPACKET_*` 상수 정의가 없으면 빌드되지 않으며, 이 저장소는 코드 열람 목적입니다.
