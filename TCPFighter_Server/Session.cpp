#include <cstdio>
#include "Session.h"
#include "Protocol.h"
#include "MakePacket.h"

std::list<st_SESSION*> g_sessionlist;

void DisconnectSession(st_SESSION* pSession)
{
	if (pSession->bIsAlive == false) return; // 이미 처리가 시작된 세션이면 중복 실행 방지

	printf("Disconnect # SessionID:%d\n", pSession->Session_id);
	pSession->bIsAlive = false;

	char deleteBuffer[sizeof(SC_DeleteCharacter)];
	int packetSize = MakePacket_SC_DeleteCharacter(deleteBuffer, pSession->Session_id);
	SendPacket_Broadcast(pSession, deleteBuffer, packetSize, false);
}

void SendPacket_Unicast(st_SESSION* pSession, char* pPacket, int size)
{
	pSession->Sendq.Enqueue(pPacket, size);
}

void SendPacket_Broadcast(st_SESSION* pFrom, char* pPacket, int size, bool bSendToSender)
{
	for (st_SESSION* pTarget : g_sessionlist)
	{
		if (!pTarget->bIsAlive) continue;

		if (!bSendToSender && pTarget->Session_id == pFrom->Session_id)
		{
			continue;
		}
		SendPacket_Unicast(pTarget, pPacket, size);
	}
}
