/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_VPY_H
#define GAME_SERVER_GAMEMODES_VPY_H
#include <game/server/gamecontroller.h>

// you can subclass GAMECONTROLLER_CTF, GAMECONTROLLER_TDM etc if you want
// todo a modification with their base as well.
class CGameController_vPy : public IGameController
{
  int m_OldMode;
  void ChangeMode(class CCharacter *pChr);
public:
	CGameController_vPy(class CGameContext *pGameServer);
	virtual void Tick();
	// add more virtual functions here if you wish
  virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponID);
  virtual void OnCharacterSpawn(class CCharacter *pChr);
  virtual bool OnEntity(int Index, vec2 Pos);
  virtual void DoWincheck();
  virtual void EndRound();
};
#endif
