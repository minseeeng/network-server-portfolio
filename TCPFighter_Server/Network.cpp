#include <cstdio>
#include <random>
#include <ws2tcpip.h>
#include "Network.h"
#include "Define.h"
#include "Protocol.h"
#include "_PacketDefine.h"
#include "PacketProc.h"
#include "MakePacket.h"

bool g_Shutdown = false;
SOCKET g_listen_sock;
DWORD g_dwSessionIDCounter = 0;

void Network()
{
	// 지연 삭제 — 이전 프레임에서 끊긴 세션은 순회가 끝난 지금 정리
	for (auto it = g_sessionlist.begin(); it != g_sessionlist.end(); )
	{
		if (!(*it)->bIsAlive)
		{
			closesocket((*it)->Socket);
			delete (*it);
			it = g_sessionlist.erase(it);
		}
		else
		{
			++it;
		}
	}

	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	FD_SET(g_listen_sock, &ReadSet);

	for (st_SESSION* pSession : g_sessionlist)
	{
		FD_SET(pSession->Socket, &ReadSet);
		// 보낼 데이터가 있는 세션만 등록 — 상시 등록하면 select가 즉시 리턴을 반복
		if (pSession->Sendq.GetUseSize() > 0)
		{
			FD_SET(pSession->Socket, &WriteSet);
		}
	}

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int iResult = select(0, &ReadSet, &WriteSet, 0, &timeout);

	if (iResult > 0)
	{
		if (FD_ISSET(g_listen_sock, &ReadSet))
		{
			netProc_Accept();
			--iResult;
		}

		for (st_SESSION* pSession : g_sessionlist)
		{
			if (iResult <= 0) break;

			bool bRead = FD_ISSET(pSession->Socket, &ReadSet);
			bool bWrite = FD_ISSET(pSession->Socket, &WriteSet);

			if (bRead && bWrite)
			{
				iResult -= 2;
				netProc_Recv(pSession);
				if (pSession->bIsAlive) netProc_Send(pSession);
			}
			else if (bRead)
			{
				--iResult;
				netProc_Recv(pSession);
			}
			else if (bWrite)
			{
				--iResult;
				netProc_Send(pSession);
			}
		}
	}
	else if (iResult == SOCKET_ERROR)
	{
		int n1 = WSAGetLastError();
		printf("select() 오류: %d\n", n1);
		g_Shutdown = true;
	}
}

void netProc_Accept()
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	SOCKET clientSocket = accept(g_listen_sock, (SOCKADDR*)&clientAddr, &addrLen);
	if (clientSocket == INVALID_SOCKET)
	{
		int a1 = WSAGetLastError();
		printf("accept 실패: %d\n", a1);
		return;
	}

	char clientIP[INET_ADDRSTRLEN];
	InetNtopA(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);

	st_SESSION* pNewSession = new st_SESSION;
	pNewSession->Session_id = ++g_dwSessionIDCounter;
	pNewSession->Socket = clientSocket;
	pNewSession->bIsAlive = true;
	pNewSession->hp = 100;
	pNewSession->Direction = dfPACKET_MOVE_DIR_RR;
	pNewSession->Action = dfACTION_IDLE;

	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<short> distribX(dfRANGE_MOVE_LEFT, dfRANGE_MOVE_RIGHT);
	std::uniform_int_distribution<short> distribY(dfRANGE_MOVE_TOP, dfRANGE_MOVE_BOTTOM);
	pNewSession->x = distribX(gen);
	pNewSession->y = distribY(gen);

	printf("Connect # IP:%s / SessionID:%d\n", clientIP, pNewSession->Session_id);
	printf("# PACKET_CONNECT # SessionID:%d\n", pNewSession->Session_id);
	printf("Create Character # SessionID:%d X:%d Y:%d\n", pNewSession->Session_id, pNewSession->x, pNewSession->y);

	char myCharBuffer[sizeof(SC_CreateMyCharacter)];
	int myCharPacketSize = MakePacket_SC_CreateMyCharacter(myCharBuffer, pNewSession->Session_id, pNewSession->Direction, pNewSession->x, pNewSession->y, pNewSession->hp);
	SendPacket_Unicast(pNewSession, myCharBuffer, myCharPacketSize);

	// 새로 접속한 클라이언트에게 "이미 접속해있던 다른 캐릭터들의 정보"를 전송
	char otherCharBuffer[sizeof(SC_OtherCharacter)];
	for (st_SESSION* pSession : g_sessionlist)
	{
		int otherCharPacketSize = MakePacket_SC_OtherCharacter(otherCharBuffer, pSession->Session_id, pSession->Direction, pSession->x, pSession->y, pSession->hp);
		SendPacket_Unicast(pNewSession, otherCharBuffer, otherCharPacketSize);
	}

	// 기존에 있던 모든 클라이언트에게 "새로운 캐릭터의 정보"를 전송
	char newCharBuffer[sizeof(SC_OtherCharacter)];
	int newCharPacketSize = MakePacket_SC_OtherCharacter(newCharBuffer, pNewSession->Session_id, pNewSession->Direction, pNewSession->x, pNewSession->y, pNewSession->hp);
	SendPacket_Broadcast(pNewSession, newCharBuffer, newCharPacketSize, false);

	g_sessionlist.push_back(pNewSession);
}

void netProc_Recv(st_SESSION* pSession)
{
	int directEnqueueSize = pSession->Recvq.DirectEnqueueSize();
	if (directEnqueueSize <= 0)
	{
		printf("ID: %d, RecvQ is full!\n", pSession->Session_id);
		DisconnectSession(pSession);
		return;
	}

	// 링버퍼의 쓰기 위치에 recv가 바로 쓰도록 해 중간 복사를 없앰
	char* pWritePtr = pSession->Recvq.GetRearBufferPtr();
	int recvResult = recv(pSession->Socket, pWritePtr, directEnqueueSize, 0);

	if (recvResult == 0)
	{
		DisconnectSession(pSession);
		return;
	}
	if (recvResult == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			// 논블로킹 소켓에서 아직 받을 데이터가 없다는 의미. 에러가 아님.
			return;
		}
		else
		{
			int r1 = WSAGetLastError();
			printf("ID: %d, recv() 에러: %d\n", pSession->Session_id, r1);
			DisconnectSession(pSession);
			return;
		}
	}

	pSession->Recvq.MoveRear(recvResult);

	while (true)
	{
		int useSize = pSession->Recvq.GetUseSize();
		if (useSize < sizeof(Header))
			break;

		Header header;
		pSession->Recvq.Peek((char*)&header, sizeof(Header));
		if (header.byCode != 0x89)
		{
			printf("ID: %d, 잘못된 패킷 코드 수신: 0x%X\n", pSession->Session_id, header.byCode);
			DisconnectSession(pSession);
			return;
		}

		int requiredSize = sizeof(Header) + header.bySize;
		if (useSize < requiredSize)
		{
			break;
		}

		char packetBuffer[4096];
		pSession->Recvq.Dequeue(packetBuffer, requiredSize);
		if (!PacketProc(pSession, header.byType, packetBuffer + sizeof(Header)))
		{
			DisconnectSession(pSession);
			return;
		}
	}
}

void netProc_Send(st_SESSION* pSession)
{
	while (pSession->Sendq.GetUseSize() > 0)
	{
		int directDequeueSize = pSession->Sendq.DirectDequeueSize();
		if (directDequeueSize <= 0)
		{
			break;
		}

		char* pReadPtr = pSession->Sendq.GetFrontBufferPtr();
		int sendResult = send(pSession->Socket, pReadPtr, directDequeueSize, 0);

		if (sendResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}
			else
			{
				printf("ID: %d, send() 에러: %d\n", pSession->Session_id, WSAGetLastError());
				DisconnectSession(pSession);
				return;
			}
		}
		// 보낸 만큼만 제거 — 남은 데이터는 다음에 쓰기 가능해지면 이어서 전송
		pSession->Sendq.MoveFront(sendResult);
	}
}
