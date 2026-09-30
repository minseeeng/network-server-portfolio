#pragma once
#include "Session.h"

bool PacketProc(st_SESSION* pSession, BYTE byPacketType, char* pPacket);
bool netPacketProc_MoveStart(st_SESSION* pSession, char* pPacket);
bool netPacketProc_MoveStop(st_SESSION* pSession, char* pPacket);
bool netPacketProc_Attack1(st_SESSION* pSession, char* pPacket);
bool netPacketProc_Attack2(st_SESSION* pSession, char* pPacket);
bool netPacketProc_Attack3(st_SESSION* pSession, char* pPacket);
