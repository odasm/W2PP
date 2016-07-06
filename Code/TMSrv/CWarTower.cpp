#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <io.h>
#include <errno.h>

#include "..\Basedef.h"
#include "..\CPSock.h"
#include "..\ItemEffect.h"
#include "Language.h"

#include "CItem.h"
#include "Server.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "ProcessClientMessage.h"
#include "ProcessDBMessage.h"
#include "CReadFiles.h"

#include "CWarTower.h"

void CWarTower::GuildProcess(tm *timeinfo)
{
	if (NewbieEventServer == 1 && timeinfo->tm_wday != 0 && timeinfo->tm_wday != 6 && timeinfo->tm_hour == GTorreHour)
	{
		if (GTorreState == 0 && timeinfo->tm_min <= 5)
		{
			sprintf(temp, g_pMessageStringTable[_DN_CHANNELWAR_BEGIN], 5);
			SendNotice(temp);
			GTorreState = 1;
			GTorreGuild = 0;
		}

		if (GTorreState == 1 && timeinfo->tm_min >= 6)
		{
			ClearArea(2445, 1850, 2546, 1920);

			GenerateMob(GTORRE, 0, 0);

			SendNotice(g_pMessageStringTable[_DN_BASEWORSTART]);
			GTorreState = 2;
		}

		//A Guerra acabou a guilda que est� defendendo a torre recebe 50 de fama.
		if (GTorreState == 2 && timeinfo->tm_min == 59)
		{
			ClearArea(2445, 1850, 2546, 1920);

			for (int i = MAX_USER; i < MAX_MOB; i++)
			{
				if (pMob[i].GenerateIndex == GTORRE)
				{
					if (pMob[i].MOB.Guild)
					{
						int Group = ServerGroup;
						int Server = pMob[i].MOB.Guild / MAX_GUILD;
						int usGuild = pMob[i].MOB.Guild;

						char guildname[256];

						BASE_GetGuildName(Group, usGuild, guildname);

						MSG_GuildInfo sm;

						sm.Type = _MSG_GuildInfo;
						sm.Size = sizeof(MSG_GuildInfo);
						sm.ID = 0;

						sm.Guild = usGuild;
						GuildInfo[usGuild].Fame += 100;

						sm.GuildInfo = GuildInfo[usGuild];

						DBServerSocket.SendOneMessage((char*)&sm, sizeof(MSG_GuildInfo));

						sprintf(temp, "etc,war_tower1 guild:%d guild_fame:%d", usGuild, GuildInfo[usGuild].Fame);
						Log(temp, "-system", 0);
					}

					DeleteMob(i, 1);
				}
			}

			GTorreState = 0;
			GTorreGuild = 0;
		}
	}
}

void CWarTower::MobKilled(int target, int conn, int PosX, int PosY)
{
	int GenerateID = pMob[target].GenerateIndex;

	if (GenerateID == GTORRE && GTorreState)
	{
		if (pMob[conn].MOB.Guild)
		{
			int Group = ServerGroup;
			int Server = pMob[conn].MOB.Guild / MAX_GUILD;
			int usGuild = pMob[conn].MOB.Guild;

			char guildname[256];

			BASE_GetGuildName(Group, usGuild, guildname);

			sprintf(temp, g_pMessageStringTable[_SS_BASEWORKILLTOWER], pMob[conn].MOB.MobName, guildname);
			SendNotice(temp);

			GTorreGuild = pMob[conn].MOB.Guild;
		}

		ClearArea(2445, 1850, 2546, 1920);

		GenerateMob(GTORRE, 0, 0);
	}
}

void CWarTower::GGenerateMob(int index, int PosX, int PosY, int tmob)
{
	if (index == GTORRE && GTorreGuild)
	{
		pMob[tmob].MOB.Guild = GTorreGuild;
		pMob[tmob].MOB.GuildLevel = 0;
	}
}

BOOL CWarTower::TowerAttack(int conn, int idx)
{
	if (pMob[idx].GenerateIndex != GTORRE)
		return TRUE;

	if (pMob[conn].MOB.Guild == 0 || pMob[conn].MOB.Guild == pMob[idx].MOB.Guild)
		return FALSE;

	return TRUE;
}