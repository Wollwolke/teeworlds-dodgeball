#include <string.h>
#include <engine/shared/config.h>
#include <engine/server.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/collision.h>
#include "db.h"

CGameControllerDB::CGameControllerDB(class CGameContext *pGameServer) 
: IGameController(pGameServer)
{
	m_ballIndex = 0;

	for (int i = 0; i < DB_MAX_BALLS; i++) 
	{
		balls[i] = 0;
	}
	
	m_pGameType = "DB";
	m_GameFlags = GAMEFLAG_TEAMS;
}

bool CGameControllerDB::OnEntity (int Index, vec2 Pos)
{	
	if (IGameController::OnEntity (Index, Pos))
		return true;
	
	//	add balls
	switch (Index)
	{
	case ENTITY_HEALTH_1: 
		if (balls[m_ballIndex] == 0)
		{
			GameServer()->m_World.InsertEntity(balls[m_ballIndex] = new CBall(&GameServer()->m_World, Pos)); 
			m_ballIndex = m_ballIndex + 1;
			break;		
		}
	default: return false;
	}
	
	return true;
}

int CGameControllerDB::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int weaponid)
{
	// do scoreing
	if (weaponid == WEAPON_SELF || weaponid == WEAPON_WORLD)
		pVictim->GetPlayer()->m_Score--;	// suicide or death-tile
	else if (pKiller && weaponid != WEAPON_GAME)
	{
		if (IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
			pKiller->m_Score--;	// teamkill
		else
			pKiller->m_Score++;	// normal kill
	}

	//	add db-score and killing message flags
	for (int i = 0; i < DB_MAX_BALLS; i++)
	{
		CBall *b = balls[i];
		if (!b || !b->m_Carrier)
			continue;

		if (b->m_Carrier == pVictim)
		{
			//	drop ball
			b->m_DropTick = Server()->Tick();
			b->m_LastCarrier = pVictim->GetPlayer();
			b->m_Carrier = 0;
			pVictim->ball = NULL;
			b->m_Vel = vec2 (0, 0);
		}
	}
	
	return 0;	
}

void CGameControllerDB::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->m_Health = 10;
	
	// give default weapons
	pChr->m_aWeapons[WEAPON_HAMMER].m_Got = 1;
	pChr->m_aWeapons[WEAPON_HAMMER].m_Ammo = -1;
	pChr->m_ActiveWeapon = WEAPON_HAMMER;
	pChr->m_LastWeapon = WEAPON_HAMMER;
}

void CGameControllerDB::OnDisconnect (const CPlayer *const p)
{
	for (int i = 0; i < DB_MAX_BALLS; ++i)
	{
		if (balls[i])
		{
			if (balls[i]->m_Carrier && p == balls[i]->m_Carrier->GetPlayer())
				balls[i]->m_Carrier = NULL;
			if (p == balls[i]->m_LastCarrier)
				balls[i]->m_LastCarrier = NULL;
		}
	}	
}

void CGameControllerDB::Tick()
{
	IGameController::Tick();
	DoTeamScoreWincheck();

	if (m_GameOverTick == -1 && m_RoundStartTick != Server()->Tick())
	{	
		//	Update balls
		for (int bi =  0; bi < DB_MAX_BALLS; ++bi)
		{
			CBall *b = balls[bi];

			if (!b)
				continue;
			
			//	Update ball position
			if (b->m_Carrier)
			{
				b->m_Pos = b->m_Carrier->m_Pos;
			}
			
			// If ball is carried by no one
			if (!b->m_Carrier)
			{	
				//Check if ball collided with any walls
				//Could be done much better.
				if (GameServer()->Collision()->IntersectLine(b->m_Pos, vec2 (b->m_Pos.x + b->m_Vel.x + 14, b->m_Pos.y + b->m_Vel.y - 14), NULL, NULL) ||
					GameServer()->Collision()->IntersectLine(b->m_Pos, vec2 (b->m_Pos.x + b->m_Vel.x + 14, b->m_Pos.y + b->m_Vel.y + 14), NULL, NULL) ||
					GameServer()->Collision()->IntersectLine(b->m_Pos, vec2 (b->m_Pos.x + b->m_Vel.x - 14, b->m_Pos.y + b->m_Vel.y - 14), NULL, NULL) ||
					GameServer()->Collision()->IntersectLine(b->m_Pos, vec2 (b->m_Pos.x + b->m_Vel.x - 14, b->m_Pos.y + b->m_Vel.y + 14), NULL, NULL) )
				{
					b->m_IsDead = 1;
				}
								
				GameServer()->Collision()->MoveBox(&b->m_Pos, &b->m_Vel, vec2 (b->ms_PhysSize, b->ms_PhysSize), 0.5f);
				
				// Do ball physics
				b->m_Vel.y += GameServer()->m_World.m_Core.m_Tuning.m_Gravity;
				if (b->m_Vel.x > -0.1f && b->m_Vel.x < 0.1f)
				{
					b->m_Vel.x = 0.0f;
				}
				else if (b->m_Vel.x < 0)
				{
					b->m_Vel.x += 0.055f;
				}
				else
				b->m_Vel.x -= 0.055f;
				
				// If ball hits death tiles, reset it
				if (GameServer()->Collision()->GetCollisionAt(static_cast<int> (b->m_Pos.x), static_cast<int> (b->m_Pos.y)) == CCollision::COLFLAG_DEATH)
				{
					GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
					b->Reset();
				}
			
				// Find all players that can grab the ball
				CCharacter *close_characters[MAX_CLIENTS];
				int max_num = GameServer()->m_World.FindEntities(b->m_Pos, b->ms_PhysSize, (CEntity**)close_characters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
				int curr_num = 0;
				for (int i = 0; i < max_num; ++i)
				{
					if (!close_characters[i]->IsAlive() || close_characters[i]->GetPlayer()->GetTeam() == -1 || GameServer()->Collision()->IntersectLine(b->m_Pos, close_characters[i]->m_Pos, NULL, NULL) ||
						balls[bi]->m_Carrier == close_characters[i]) // don't take flag if already have one
						continue;
					close_characters[curr_num++] = close_characters[i];
				}
				
				if (!curr_num)
					continue;

				CCharacter *ch = close_characters[Server()->Tick() % curr_num];
				
				if (b->m_IsDead && !ch->ball) {
				  ch->CaptureBall(b);	
				}
				
				// If the ball is live and was thrown from someone on the opposite team
				if (b->m_IsDead == 0 && ch->GetPlayer()->m_Team != b->m_LastCarrier->m_Team)
				{
					GameServer()->CreateSound(b->m_Pos, SOUND_HIT);
					m_aTeamscore[b->m_LastCarrier->m_Team]++;
					b->m_LastCarrier->m_Score++;
					b->m_IsDead = 1;
					ch->Die(b->m_LastCarrier->m_ClientID, WEAPON_GAME);
				}
			}
		}
	}
	
}

void CGameControllerDB::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;
		
	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];

	pGameDataObj->m_FlagCarrierBlue = 0;
	pGameDataObj->m_FlagCarrierRed = 0;
	
}

//	Ball
CBall::CBall (CGameWorld *pGameWorld, vec2 _pos)
:	CEntity (pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_ProximityRadius = ms_PhysSize;
	m_StandPos = _pos;
	Reset();
}

void CBall::Reset()
{
	m_Carrier = 0;
	m_LastCarrier = 0;
	m_Pos = m_StandPos;
	m_Vel = vec2 (0, 0);
	m_DropTick = Server()->Tick();
	m_IsDead = 1;
}

void CBall::Snap(int SnappingClient)
{	
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *ball = (CNetObj_Pickup *)Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup));
	
	if(!ball)
		return;
		
	if (!m_Carrier) {	
		ball->m_X = (int)m_Pos.x;
		ball->m_Y = (int)m_Pos.y;
	}
}

void CBall::Tick()
{
	//Handle ball <-> ball collision
	for (int i =  0; i < DB_MAX_BALLS; ++i)
	{
		CBall* b = static_cast<CGameControllerDB *>(GameServer()->m_pController)->balls[i];
		
		if (!b)
			continue;
		
		//Don't nudge the same ball
		if (b == this)
			continue;
		
		//Only do ball collision if ball is not carried by a player
		if (!b->m_Carrier)
		{
			float Distance = distance(m_Pos, b->m_Pos);
			vec2 Dir = normalize(m_Pos - b->m_Pos);
			if(Distance < ms_PhysSize*1.85f && Distance > 0.0f)
			{
				float a = (ms_PhysSize*1.8f - Distance);
				float Velocity = 0.1f;

				// make sure that we don't add excess force by checking the
				// direction against the current velocity. if not zero.
				if (length(m_Vel) > 0.0001)
					Velocity = 1-(dot(normalize(m_Vel), Dir)+1)/2;

				m_Vel += Dir*a*(Velocity*0.95f);
				m_Vel *= 0.99f;
			}
		}
	}
	
	//Handle ball vs. player collision
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
			    
		CPlayer *p = GameServer()->m_apPlayers[i];
		if (!p)
			continue;
			
		CCharacter *ch = GameServer()->m_apPlayers[i]->GetCharacter();
		if(!ch)
			continue;
		
		float Distance = distance(m_Pos, ch->m_Pos);
		vec2 Dir = normalize(m_Pos - ch->m_Pos);
		if(Distance < ms_PhysSize*1.85f && Distance > 0.0f)
		{
			float a = (ms_PhysSize*1.8f - Distance);
			float Velocity = 0.1f;

			// make sure that we don't add excess force by checking the
			// direction against the current velocity. if not zero.
			if (length(m_Vel) > 0.0001)
				Velocity = 1-(dot(normalize(m_Vel), Dir)+1)/2;

			m_Vel += Dir*a*(Velocity*0.95f);
			m_Vel *= 0.99f;
		}
	}
}










