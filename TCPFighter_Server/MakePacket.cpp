#include <cstring>
#include "MakePacket.h"
#include "Protocol.h"
#include "_PacketDefine.h"

int MakePacket_SC_CreateMyCharacter(char* buffer, DWORD id, BYTE direction, short x, short y, char hp)
{
	SC_CreateMyCharacter pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_CreateMyCharacter) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	pkt.id = id;
	pkt.direction = direction;
	pkt.x = x;
	pkt.y = y;
	pkt.hp = hp;

	memcpy(buffer, &pkt, sizeof(SC_CreateMyCharacter));

	return sizeof(SC_CreateMyCharacter);
}

int MakePacket_SC_OtherCharacter(char* buffer, DWORD id, BYTE direction, short x, short y, char hp)
{
	SC_OtherCharacter pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_OtherCharacter) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	pkt.id = id;
	pkt.direction = direction;
	pkt.x = x;
	pkt.y = y;
	pkt.hp = hp;

	memcpy(buffer, &pkt, sizeof(SC_OtherCharacter));

	return sizeof(SC_OtherCharacter);
}

int MakePacket_SC_MoveStart(char* buffer, DWORD id, BYTE byDirection, short x, short y)
{
	SC_MoveStart pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_MoveStart) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_MOVE_START;
	pkt.id = id;
	pkt.byDirection = byDirection;
	pkt.x = x;
	pkt.y = y;

	memcpy(buffer, &pkt, sizeof(SC_MoveStart));

	return sizeof(SC_MoveStart);
}

int MakePacket_SC_Damage(char* buffer, DWORD attackerID, DWORD damagedID, char damagedHP)
{
	SC_Damage pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_Damage) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_DAMAGE;
	pkt.Attack_id = attackerID;
	pkt.Damage_id = damagedID;
	pkt.damage_hp = damagedHP;

	memcpy(buffer, &pkt, sizeof(SC_Damage));

	return sizeof(SC_Damage);
}

int MakePacket_SC_MoveStop(char* buffer, DWORD id, BYTE byDirection, short x, short y)
{
	SC_MoveStop pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_MoveStop) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_MOVE_STOP;
	pkt.id = id;
	pkt.byDirection = byDirection;
	pkt.x = x;
	pkt.y = y;

	memcpy(buffer, &pkt, sizeof(SC_MoveStop));

	return sizeof(SC_MoveStop);
}

int MakePacket_SC_DeleteCharacter(char* buffer, DWORD id)
{
	SC_DeleteCharacter pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_DeleteCharacter) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_DELETE_CHARACTER;
	pkt.id = id;

	memcpy(buffer, &pkt, sizeof(SC_DeleteCharacter));

	return sizeof(SC_DeleteCharacter);
}

int MakePacket_SC_Attack_1(char* buffer, DWORD id, BYTE byDirection, short x, short y)
{
	SC_Attack_1 pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_Attack_1) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_ATTACK1;
	pkt.id = id;
	pkt.byDirection = byDirection;
	pkt.x = x;
	pkt.y = y;

	memcpy(buffer, &pkt, sizeof(SC_Attack_1));

	return sizeof(SC_Attack_1);
}

int MakePacket_SC_Attack_2(char* buffer, DWORD id, BYTE byDirection, short x, short y)
{
	SC_Attack_2 pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_Attack_2) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_ATTACK2;
	pkt.id = id;
	pkt.byDirection = byDirection;
	pkt.x = x;
	pkt.y = y;

	memcpy(buffer, &pkt, sizeof(SC_Attack_2));

	return sizeof(SC_Attack_2);
}

int MakePacket_SC_Attack_3(char* buffer, DWORD id, BYTE byDirection, short x, short y)
{
	SC_Attack_3 pkt;
	pkt.header.byCode = 0x89;
	pkt.header.bySize = sizeof(SC_Attack_3) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_ATTACK3;
	pkt.id = id;
	pkt.byDirection = byDirection;
	pkt.x = x;
	pkt.y = y;

	memcpy(buffer, &pkt, sizeof(SC_Attack_3));

	return sizeof(SC_Attack_3);
}
