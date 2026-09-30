#pragma once
#include <winsock2.h>

int MakePacket_SC_CreateMyCharacter(char* buffer, DWORD id, BYTE direction, short x, short y, char hp);
int MakePacket_SC_OtherCharacter(char* buffer, DWORD id, BYTE direction, short x, short y, char hp);
int MakePacket_SC_DeleteCharacter(char* buffer, DWORD id);
int MakePacket_SC_MoveStop(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_MoveStart(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Attack_1(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Attack_2(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Attack_3(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Damage(char* buffer, DWORD attackerID, DWORD damagedID, char damagedHP);
