#include <cstdio>
#include <cstdlib>
#include "PacketProc.h"
#include "Define.h"
#include "Protocol.h"
#include "_PacketDefine.h"
#include "MakePacket.h"

bool PacketProc(st_SESSION* pSession, BYTE byPacketType, char* pPacket)
{
	printf("Packet Received # SessionID:%d / PacketType:%d\n", pSession->Session_id, byPacketType);

	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
		return netPacketProc_MoveStart(pSession, pPacket);
	case dfPACKET_CS_MOVE_STOP:
		return netPacketProc_MoveStop(pSession, pPacket);
	case dfPACKET_CS_ATTACK1:
		return netPacketProc_Attack1(pSession, pPacket);
	case dfPACKET_CS_ATTACK2:
		return netPacketProc_Attack2(pSession, pPacket);
	case dfPACKET_CS_ATTACK3:
		return netPacketProc_Attack3(pSession, pPacket);
	default:
		return false;
	}
}

bool netPacketProc_MoveStart(st_SESSION* pSession, char* pPacket)
{
	CS_MoveStart* pRecvPacket = (CS_MoveStart*)pPacket;

	printf("# PACKET_MOVESTART # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
		pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);

	if (abs(pSession->x - pRecvPacket->x) > dfERROR_RANGE ||
		abs(pSession->y - pRecvPacket->y) > dfERROR_RANGE)
	{
		pSession->x = pRecvPacket->x;
		pSession->y = pRecvPacket->y;
	}

	pSession->Action = pRecvPacket->byDirection;

	switch (pRecvPacket->byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pSession->Direction = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pSession->Direction = dfPACKET_MOVE_DIR_LL;
		break;
	}

	char sendBuffer[sizeof(SC_MoveStart)];
	int packetSize = MakePacket_SC_MoveStart(sendBuffer, pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);
	SendPacket_Broadcast(pSession, sendBuffer, packetSize, false);

	return true;
}

bool netPacketProc_MoveStop(st_SESSION* pSession, char* pPacket)
{
	CS_MoveStop* pRecvPacket = (CS_MoveStop*)pPacket;
	printf("# PACKET_MOVESTOP # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
		pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);

	if (abs(pSession->x - pRecvPacket->x) > dfERROR_RANGE ||
		abs(pSession->y - pRecvPacket->y) > dfERROR_RANGE)
	{
		pSession->x = pRecvPacket->x;
		pSession->y = pRecvPacket->y;
	}

	pSession->Action = dfACTION_IDLE;
	pSession->Direction = pRecvPacket->byDirection;
	pSession->x = pRecvPacket->x;
	pSession->y = pRecvPacket->y;

	char sendBuffer[sizeof(SC_MoveStop)];
	int packetSize = MakePacket_SC_MoveStop(sendBuffer, pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);
	SendPacket_Broadcast(pSession, sendBuffer, packetSize, false);

	return true;
}

bool netPacketProc_Attack1(st_SESSION* pSession, char* pPacket)
{
	CS_Attack_1* pRecvPacket = (CS_Attack_1*)pPacket;
	printf("# PACKET_ATTACK1 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
		pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);

	pSession->Action = dfPACKET_CS_ATTACK1;
	pSession->Direction = pRecvPacket->byDirection;
	pSession->x = pRecvPacket->x;
	pSession->y = pRecvPacket->y;

	char sendBuffer[sizeof(SC_Attack_1)];
	int packetSize = MakePacket_SC_Attack_1(sendBuffer, pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);
	SendPacket_Broadcast(pSession, sendBuffer, packetSize, false);

	return true;
}

bool netPacketProc_Attack2(st_SESSION* pSession, char* pPacket)
{
	CS_Attack_2* pRecvPacket = (CS_Attack_2*)pPacket;
	printf("# PACKET_ATTACK2 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
		pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);

	pSession->Action = dfPACKET_CS_ATTACK2;
	pSession->Direction = pRecvPacket->byDirection;
	pSession->x = pRecvPacket->x;
	pSession->y = pRecvPacket->y;

	char sendBuffer[sizeof(SC_Attack_2)];
	int packetSize = MakePacket_SC_Attack_2(sendBuffer, pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);
	SendPacket_Broadcast(pSession, sendBuffer, packetSize, false);

	return true;
}

bool netPacketProc_Attack3(st_SESSION* pSession, char* pPacket)
{
	CS_Attack_3* pRecvPacket = (CS_Attack_3*)pPacket;
	printf("# PACKET_ATTACK3 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
		pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);

	pSession->Action = dfPACKET_CS_ATTACK3;
	pSession->Direction = pRecvPacket->byDirection;
	pSession->x = pRecvPacket->x;
	pSession->y = pRecvPacket->y;

	char sendBuffer[sizeof(SC_Attack_3)];
	int packetSize = MakePacket_SC_Attack_3(sendBuffer, pSession->Session_id, pRecvPacket->byDirection, pRecvPacket->x, pRecvPacket->y);
	SendPacket_Broadcast(pSession, sendBuffer, packetSize, false);

	return true;
}
