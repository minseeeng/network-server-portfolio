#include <cstdio>
#include <winsock2.h>
#include <Windows.h>
#include "Define.h"
#include "Network.h"
#include "Update.h"

#pragma comment(lib,"ws2_32")
#pragma comment(lib,"Winmm.lib")

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
	const int GAME_FRAME_PER_SEC = 50;
	DWORD dwFrameTime = 1000 / GAME_FRAME_PER_SEC;

	// 네트워크는 매 루프 처리하고, 게임 로직은 20ms(50fps)마다 한 번씩 갱신
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
