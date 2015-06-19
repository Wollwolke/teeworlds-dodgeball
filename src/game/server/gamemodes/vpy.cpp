/* v.py by Getkey. Some stuff is from zCatch by erd and Teetime (a diff can easily tell you what!).
 * See licence.txt in the root of the distribution for more information.
 */

#include <engine/shared/config.h>//server config
#include <game/server/player.h>//GameServer()
#include <game/server/entities/pickup.h>//hearts
#include "vpy.h"

CGameController_vPy::CGameController_vPy(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "v.py";
	m_OldMode = g_Config.m_SvMode;
	g_Config.m_SvScorelimit = 10;
}

void CGameController_vPy::Tick() {

	IGameController::Tick();

	if(m_OldMode != g_Config.m_SvMode)
	{
		//change everybody's weapon
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetCharacter())
			{
				GameServer()->m_apPlayers[i]->GetCharacter()->RemoveWeapons();
				ChangeMode(GameServer()->m_apPlayers[i]->GetCharacter());
				if(g_Config.m_SvMode != 5)
				{//not ninja
					GameServer()->m_apPlayers[i]->GetCharacter()->SetWeapon();
				}
			}
		}
		m_OldMode = g_Config.m_SvMode;
	}
}

int CGameController_vPy::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) {
	if(!pKiller || Weapon == WEAPON_GAME || (Weapon == WEAPON_WORLD && pVictim->GetPlayer()->m_Chatprotected))
		return 0;

	if (Weapon != WEAPON_SELF && Weapon != WEAPON_WORLD)// Normal death
	{
		pVictim->IncreaseHealth(-1);
		
		if(g_Config.m_SvChatkillPenalty != 0 && pVictim->GetPlayer()->m_Chatprotected) {//chatkill
			pKiller->GetCharacter()->IncreaseHealth(-g_Config.m_SvChatkillPenalty);

			char Buf[23];
			str_format(Buf, sizeof(Buf), "Chatkill: -%d point(s)", g_Config.m_SvChatkillPenalty);
			GameServer()->SendBroadcast(Buf, pKiller->GetCID());
		}

		CPickup *H = new CPickup(&GameServer()->m_World, POWERUP_HEALTH);
		H->m_Pos = pVictim->m_Pos;
	} else {// Suicide
		pVictim->IncreaseHealth(-g_Config.m_SvKillPenalty);
	}

	pVictim->GetPlayer()->m_Score = pVictim->m_Health;
	return 0;

}

void CGameController_vPy::ChangeMode(class CCharacter *pChr)
{
	switch(g_Config.m_SvMode)
	{
	case 1: /* Instagib - Only Riffle */
		pChr->GiveWeapon(WEAPON_RIFLE, -1);
		break;
	case 2: /* All Weapons */
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		pChr->GiveWeapon(WEAPON_GUN, g_Config.m_SvWeaponsAmmo);
		pChr->GiveWeapon(WEAPON_GRENADE, g_Config.m_SvWeaponsAmmo);
		pChr->GiveWeapon(WEAPON_SHOTGUN, g_Config.m_SvWeaponsAmmo);
		pChr->GiveWeapon(WEAPON_RIFLE, g_Config.m_SvWeaponsAmmo);
		break;
	case 3: /* Hammer */
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		break;
	case 4: /* Grenade */
		pChr->GiveWeapon(WEAPON_GRENADE, g_Config.m_SvGrenadeEndlessAmmo ? -1 : g_Config.m_SvWeaponsAmmo);
		break;
	case 5: /* Ninja */
		pChr->GiveNinja();
		break;
	}
}

void CGameController_vPy::OnCharacterSpawn(class CCharacter *pChr)
{
	// give default weapons
	ChangeMode(pChr);
	if(pChr->m_Health == 0)//only appen on connection
	{//shitty hack
		pChr->m_Health = 1;
		pChr->GetPlayer()->m_Score = 1;
	}

}

bool CGameController_vPy::OnEntity(int Index, vec2 Pos)
{
	if(Index == ENTITY_SPAWN)
		m_aaSpawnPoints[0][m_aNumSpawnPoints[0]++] = Pos;
	else if(Index == ENTITY_SPAWN_RED)
		m_aaSpawnPoints[1][m_aNumSpawnPoints[1]++] = Pos;
	else if(Index == ENTITY_SPAWN_BLUE)
		m_aaSpawnPoints[2][m_aNumSpawnPoints[2]++] = Pos;

	return false;
}
