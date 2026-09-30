#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <random>
#include "RingBuffer.h"
#include "_PacketDefine.h"
#include <list>

#pragma comment(lib,"ws2_32")
#pragma comment(lib,"Winmm.lib") 

#define SERVERPORT 5000
//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define MOVE_SPEED_X 3
#define MOVE_SPEED_Y 2

//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE		50

//---------------------------------------------------------------
// 화면 이동 영역
//---------------------------------------------------------------
#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

//---------------------------------------------------------------
// 공격범위.
//---------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		30
#define dfATTACK2_RANGE_Y		30
#define dfATTACK3_RANGE_Y		40

#define dfACTION_IDLE			99

#pragma pack(1)
struct Header
{
	BYTE	byCode; 	// 패킷코드 0x89 고정.
	BYTE	bySize; 	// 패킷 사이즈.
	BYTE	byType; 	// 패킷타입.
};

struct SC_CreateMyCharacter
{
	Header header;
	DWORD id;
	BYTE direction; //방향
	short x;
	short y;
	char hp;
};

struct SC_OtherCharacter
{
	Header header;
	DWORD id;
	BYTE direction; //방향
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

/// <클라이언트->서버>
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

bool g_Shutdown = false;
SOCKET g_listen_sock;
std::list<st_SESSION*>g_sessionlist;
DWORD g_dwSessionIDCounter = 0;

void Network();
void Update();
void netProc_Accept();
void netProc_Recv(st_SESSION* pSession);
void netProc_Send(st_SESSION* pSession);
void DisconnectSession(st_SESSION* pSession);
bool PacketProc(st_SESSION* pSession, BYTE byPacketType, char* pPacket);
bool netPacketProc_MoveStart(st_SESSION* pSession, char* pPacket);
bool netPacketProc_MoveStop(st_SESSION* pSession, char* pPacket);
bool netPacketProc_Attack1(st_SESSION* pSession, char* pPacket);
bool netPacketProc_Attack2(st_SESSION* pSession, char* pPacket);
bool netPacketProc_Attack3(st_SESSION* pSession, char* pPacket);
void SendPacket_Unicast(st_SESSION* pSession, char* pPacket, int size);
void SendPacket_Broadcast(st_SESSION* pFrom, char* pPacket, int size, bool bSendToSender);


int MakePacket_SC_CreateMyCharacter(char* buffer, DWORD id, BYTE direction, short x, short y, char hp);
int MakePacket_SC_OtherCharacter(char* buffer, DWORD id, BYTE direction, short x, short y, char hp);
int MakePacket_SC_DeleteCharacter(char* buffer, DWORD id);
int MakePacket_SC_MoveStop(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_MoveStart(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Attack_1(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Attack_2(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Attack_3(char* buffer, DWORD id, BYTE byDirection, short x, short y);
int MakePacket_SC_Damage(char* buffer, DWORD attackerID, DWORD damagedID, char damagedHP);

int main()
{
	timeBeginPeriod(1);
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		int e1 = WSAGetLastError();
		printf("WSAStartup() 오류 : %d", e1);
		return -1;
	}
	printf("WSAStartup #\n");
	//리슨 소켓 생성
	g_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_listen_sock == INVALID_SOCKET)
	{
		int e2 = WSAGetLastError();
		printf("socket() 오류: %d\n", e2);
		WSACleanup();
		return -1;
	}


	u_long on = 1;
	if (ioctlsocket(g_listen_sock, FIONBIO, &on) == SOCKET_ERROR)
	{
		int e4 = WSAGetLastError();
		printf("리슨 소켓 논블로킹 설정 실패.,%d", e4);
		closesocket(g_listen_sock);
		WSACleanup();
		return -1;
	}


	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int retval;
	retval = bind(g_listen_sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
	{
		int e5 = WSAGetLastError();
		printf("바인딩 실패.%d\n", e5);
		closesocket(g_listen_sock);
		WSACleanup();
		return -1;
	}
	printf("Bind OK # Port:%d\n", SERVERPORT);

	//리슨시작
	retval = listen(g_listen_sock, SOMAXCONN);
	int e3 = WSAGetLastError();
	if (retval == SOCKET_ERROR)
	{
		printf("listen() 오류: %d\n", e3);
		closesocket(g_listen_sock);
		WSACleanup();
		return -1;
	}
	printf("Listen OK #\n");

	DWORD dwOldTick = timeGetTime();
	const int GAME_FRAME_PER_SEC = 50; // 50fps로 설정
	DWORD dwFrameTime = 1000 / GAME_FRAME_PER_SEC;

	while (!g_Shutdown)
	{
		Network();
		DWORD dwCurrentTick = timeGetTime();
		if (dwCurrentTick - dwOldTick >= dwFrameTime)
		{
			Update();
			dwOldTick = dwCurrentTick;
		}
	}

	for (st_SESSION* pSession : g_sessionlist)
	{
		closesocket(pSession->Socket);
		delete pSession;
	}
	g_sessionlist.clear();
	closesocket(g_listen_sock);
	WSACleanup();
	timeEndPeriod(1);


	return 0;
}

void Network()
{
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

	char* pWritePtr = pSession->Recvq.GetRearBufferPtr();
	int recvResult = recv(pSession->Socket, pWritePtr, directEnqueueSize, 0);

	// 2. recv() 결과 처리
	if (recvResult == 0) // 정상 종료 
	{
		// 클라이언트가 정상적으로 소켓를 닫고 접속을 종료함
		DisconnectSession(pSession);
		return;
	}
	if (recvResult == SOCKET_ERROR) // 소켓 에러 
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

	// 3. 받은 만큼 링버퍼의 쓰기 포인터 이동
	pSession->Recvq.MoveRear(recvResult);

	while (true)
	{
		int useSize = pSession->Recvq.GetUseSize();
		// 최소한 헤더 크기만큼의 데이터가 없으면 중지
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
		pSession->Sendq.MoveFront(sendResult);
	}
}


bool PacketProc(st_SESSION* pSession, BYTE byPacketType, char* pPacket)
{
	printf("Packet Received # SessionID:%d / PacketType:%d\n", pSession->Session_id, byPacketType);

	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
		return netPacketProc_MoveStart(pSession, pPacket);
		break;
	case dfPACKET_CS_MOVE_STOP:
		return netPacketProc_MoveStop(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK1:
		return netPacketProc_Attack1(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK2:
		return netPacketProc_Attack2(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK3:
		return netPacketProc_Attack3(pSession, pPacket);
		break;

	default:
		return false;

	}
	return TRUE;

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

	pSession->Action = dfACTION_IDLE; // 현재 액션을 '정지(Idle)'로 설정
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

	pSession->Action = dfPACKET_CS_ATTACK1; // 현재 액션을 '공격1'로 설정
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

void Update()
{
	const char* actionNames[] = { "LL", "LU", "UU", "RU", "RR", "RD", "DD", "LD" };
	for (st_SESSION* pSession : g_sessionlist)
	{
		if (!pSession->bIsAlive) 
			continue;
		if (pSession->hp <= 0)
		{
			DisconnectSession(pSession);
			continue;
		}

		bool moved = false;
		switch (pSession->Action)
		{
		
		case dfPACKET_MOVE_DIR_LL: pSession->x -= MOVE_SPEED_X; moved = true; break;
		case dfPACKET_MOVE_DIR_LU: pSession->x -= MOVE_SPEED_X; pSession->y -= MOVE_SPEED_Y; moved = true; break;
		case dfPACKET_MOVE_DIR_UU: pSession->y -= MOVE_SPEED_Y; moved = true; break;
		case dfPACKET_MOVE_DIR_RU: pSession->x += MOVE_SPEED_X; pSession->y -= MOVE_SPEED_Y; moved = true; break;
		case dfPACKET_MOVE_DIR_RR: pSession->x += MOVE_SPEED_X; moved = true; break;
		case dfPACKET_MOVE_DIR_RD: pSession->x += MOVE_SPEED_X; pSession->y += MOVE_SPEED_Y; moved = true; break;
		case dfPACKET_MOVE_DIR_DD: pSession->y += MOVE_SPEED_Y; moved = true; break;
		case dfPACKET_MOVE_DIR_LD: pSession->x -= MOVE_SPEED_X; pSession->y += MOVE_SPEED_Y; moved = true; break;
		
		case dfPACKET_CS_ATTACK1:
		case dfPACKET_CS_ATTACK2:
		case dfPACKET_CS_ATTACK3:
		{
			int rangeX = 0, rangeY = 0;
			if (pSession->Action == dfPACKET_CS_ATTACK1) { rangeX = dfATTACK1_RANGE_X; rangeY = dfATTACK1_RANGE_Y; }
			if (pSession->Action == dfPACKET_CS_ATTACK2) { rangeX = dfATTACK2_RANGE_X; rangeY = dfATTACK2_RANGE_Y; }
			if (pSession->Action == dfPACKET_CS_ATTACK3) { rangeX = dfATTACK3_RANGE_X; rangeY = dfATTACK3_RANGE_Y; }

			for (st_SESSION* pTarget : g_sessionlist)
			{
				if (!pTarget->bIsAlive || pSession->Session_id == pTarget->Session_id || pTarget->hp <= 0) 
					continue;
				if (abs(pSession->y - pTarget->y) > rangeY) 
					continue;

				bool bHit = false;
				if (pSession->Direction == dfPACKET_MOVE_DIR_RR)
				{
					if (pTarget->x > pSession->x && pTarget->x <= pSession->x + rangeX) bHit = true;
				}
				else
				{
					if (pTarget->x < pSession->x && pTarget->x >= pSession->x - rangeX) bHit = true;
				}

				if (bHit)
				{
					pTarget->hp -= 2;
					if (pTarget->hp < 0) 
						pTarget->hp = 0;

					
					printf("# AttackHit # AttackerID:%d -> TargetID:%d / TargetHP:%d\n",
						pSession->Session_id, pTarget->Session_id, pTarget->hp);
			

					char damageBuffer[sizeof(SC_Damage)];
					int packetSize = MakePacket_SC_Damage(damageBuffer, pSession->Session_id, pTarget->Session_id, pTarget->hp);
					SendPacket_Broadcast(pSession, damageBuffer, packetSize, true);
					break;
				}
			}
			pSession->Action = dfACTION_IDLE;
		}
		break;
		}
		
		if (moved)
		{
			// 화면 경계 처리
			if (pSession->x < dfRANGE_MOVE_LEFT)
			{
				pSession->x = dfRANGE_MOVE_LEFT;
				pSession->Action = dfACTION_IDLE; 
			}
			if (pSession->x > dfRANGE_MOVE_RIGHT)
			{
				pSession->x = dfRANGE_MOVE_RIGHT;
				pSession->Action = dfACTION_IDLE; 
			}
			if (pSession->y < dfRANGE_MOVE_TOP)
			{
				pSession->y = dfRANGE_MOVE_TOP;
				pSession->Action = dfACTION_IDLE; 
			}
			if (pSession->y > dfRANGE_MOVE_BOTTOM)
			{
				pSession->y = dfRANGE_MOVE_BOTTOM;
				pSession->Action = dfACTION_IDLE; 
			}

	
			if (pSession->Action != dfACTION_IDLE)
			{
				printf("# gameRun:%s # SessionID:%d / X:%d / Y:%d\n",
					actionNames[pSession->Action], pSession->Session_id, pSession->x, pSession->y);
			}
		}
		
	}
}

void DisconnectSession(st_SESSION* pSession)
{
	if (pSession->bIsAlive == false) return; // 이미 처리가 시작된 세션이면 중복 실행 방지

	printf("Disconnect # SessionID:%d\n", pSession->Session_id);
	pSession->bIsAlive = false;


	char deleteBuffer[sizeof(SC_DeleteCharacter)];
	int packetSize = MakePacket_SC_DeleteCharacter(deleteBuffer, pSession->Session_id);
	SendPacket_Broadcast(pSession, deleteBuffer, packetSize, false);
}

// -----------------------------------------------------------------
// 패킷 생성 함수들
// -----------------------------------------------------------------

int MakePacket_SC_CreateMyCharacter(char* buffer, DWORD id, BYTE direction, short x, short y, char hp)
{
	SC_CreateMyCharacter pkt;
	pkt.header.byCode = 0x89;
	// 중요: 이제 사이즈는 페이로드(데이터)의 크기입니다.
	pkt.header.bySize = sizeof(SC_CreateMyCharacter) - sizeof(Header);
	pkt.header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	pkt.id = id;
	pkt.direction = direction;
	pkt.x = x;
	pkt.y = y;
	pkt.hp = hp;

	// 완성된 패킷 구조체를 버퍼에 복사
	memcpy(buffer, &pkt, sizeof(SC_CreateMyCharacter));

	return sizeof(SC_CreateMyCharacter);
}

// SC_OtherCharacter 패킷 생성 함수
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

// SC_MoveStart 패킷 생성 함수
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

// SC_Damage 패킷 생성 함수
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