#pragma once
#include <winsock2.h>
#include "Session.h"

extern bool g_Shutdown;
extern SOCKET g_listen_sock;

void Network();
void netProc_Accept();
void netProc_Recv(st_SESSION* pSession);
void netProc_Send(st_SESSION* pSession);
