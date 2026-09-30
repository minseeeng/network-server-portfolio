# TCPFighter Server — 소스 코드

select 모델 기반 실시간 2D 대전 게임 서버의 핵심 구현입니다. (1인 단독 구현)

| 파일 | 내용 |
|---|---|
| `main.cpp` | 리슨 소켓 초기화 · 메인 루프 (네트워크는 매 루프, 게임 로직은 20ms마다) |
| `Network.h/.cpp` | select 멀티플렉싱 · Accept / Recv(패킷 프레이밍) / Send(부분 전송) · 지연 삭제 |
| `Session.h/.cpp` | 세션 구조체 · 접속 종료 처리(중복 방지) · 유니캐스트/브로드캐스트 |
| `PacketProc.h/.cpp` | 클라이언트 패킷 처리 (이동 시작·정지, 공격 1~3) |
| `Update.h/.cpp` | 서버 측 판정 — 이동 계산 · 화면 경계 제한 · 공격 범위 판정과 데미지 |
| `MakePacket.h/.cpp` | 서버 → 클라이언트 패킷 생성 |
| `Protocol.h` | 패킷 헤더와 패킷 구조체 |
| `Define.h` | 서버 포트 · 이동 속도 · 이동 영역 · 공격 범위 상수 |
| `RingBuffer.h` / `ringbuffer.cpp` | 직접 구현한 링버퍼 — `DirectEnqueueSize`로 `recv()` 직접 적재(무복사 수신 경로) |

> **빌드 안내** — 패킷 타입·방향 상수를 정의한 헤더(`_PacketDefine.h`)는 교육기관 제공 자료라 저작권을 존중해 저장소에서 제외했습니다. 해당 헤더의 `dfPACKET_*` 상수 정의가 없으면 빌드되지 않으며, 이 저장소는 코드 열람 목적입니다.
