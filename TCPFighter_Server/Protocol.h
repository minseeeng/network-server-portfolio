#pragma once
#include <winsock2.h>

#pragma pack(1)
struct Header
{
	BYTE	byCode; 	// 패킷코드 0x89 고정
	BYTE	bySize; 	// 헤더를 뺀 페이로드 크기
	BYTE	byType; 	// 패킷타입
};

//-----------------------------------------------------------------
// 서버 -> 클라이언트
//-----------------------------------------------------------------
struct SC_CreateMyCharacter
{
	Header header;
	DWORD id;
	BYTE direction;
	short x;
	short y;
	char hp;
};

struct SC_OtherCharacter
{
	Header header;
	DWORD id;
	BYTE direction;
	short x;
	short y;
	char hp;
};

struct SC_DeleteCharacter
{
	Header header;
	DWORD id;
};

struct SC_MoveStop
{
	Header header;
	DWORD id;
	BYTE byDirection;
	short x;
	short y;
};

struct SC_MoveStart
{
	Header header;
	DWORD id;
	BYTE byDirection;
	short x;
	short y;
};

struct SC_Attack_1
{
	Header header;
	DWORD id;
	BYTE byDirection;
	short x;
	short y;
};

struct SC_Attack_2
{
	Header header;
	DWORD id;
	BYTE byDirection;
	short x;
	short y;
};

struct SC_Attack_3
{
	Header header;
	DWORD id;
	BYTE byDirection;
	short x;
	short y;
};

struct SC_Damage
{
	Header header;
	DWORD Attack_id;
	DWORD Damage_id;
	char damage_hp;
};

//-----------------------------------------------------------------
// 클라이언트 -> 서버
//-----------------------------------------------------------------
struct CS_MoveStop
{
	BYTE byDirection;
	short x;
	short y;
};

struct CS_MoveStart
{
	BYTE byDirection;
	short x;
	short y;
};

struct CS_Attack_1
{
	BYTE byDirection;
	short x;
	short y;
};

struct CS_Attack_2
{
	BYTE byDirection;
	short x;
	short y;
};

struct CS_Attack_3
{
	BYTE byDirection;
	short x;
	short y;
};
#pragma pack()
