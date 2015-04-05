#include <game/server/gamecontroller.h>
#include <game/server/entity.h>

enum {
	DB_MAX_BALLS = 512
};

class CGameControllerDB : public IGameController
{
public:
	class CBall *balls[DB_MAX_BALLS];
	int m_ballIndex;
	
	CGameControllerDB(class CGameContext *pGameServer);
	
	virtual void Tick();
	virtual bool OnEntity (int Index, vec2 Pos);
	virtual int OnCharacterDeath (class CCharacter *pVictim, class CPlayer *pKiller, int weapon);
	virtual void OnCharacterSpawn (class CCharacter *chr);
	virtual void OnDisconnect (const CPlayer *const p);
	virtual void Snap(int SnappingClient);
};

class CBall : public CEntity
{
public:
	static const int ms_PhysSize = 14;
	CCharacter *m_Carrier;
	CPlayer *m_LastCarrier;
	vec2 m_Vel;
	vec2 m_StandPos;
	int m_IsDead;
	int m_DropTick;
	
	CBall (CGameWorld *pGameWorld, vec2 Pos);

	virtual void Reset();
	virtual void Snap(int SnappingClient);
	virtual void Tick();
};

