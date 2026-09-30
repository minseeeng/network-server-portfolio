#include <cstdio>
#include <cstdlib>
#include "Update.h"
#include "Session.h"
#include "Define.h"
#include "Protocol.h"
#include "_PacketDefine.h"
#include "MakePacket.h"

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
