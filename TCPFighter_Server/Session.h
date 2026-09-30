#pragma once
#include <winsock2.h>
#include <list>
#include "RingBuffer.h"

struct st_SESSION
{
	SOCKET Socket;
	DWORD Session_id;
	RingBuffer Recvq;
	RingBuffer Sendq;

	bool bIsAlive;
	DWORD Action;
	BYTE Direction;
	short x;
	short y;
	char hp;
};

extern std::list<st_SESSION*> g_sessionlist;

void DisconnectSession(st_SESSION* pSession);
void SendPacket_Unicast(st_SESSION* pSession, char* pPacket, int size);
void SendPacket_Broadcast(st_SESSION* pFrom, char* pPacket, int size, bool bSendToSender);
