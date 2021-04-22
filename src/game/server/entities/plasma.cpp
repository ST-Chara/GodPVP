/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "plasma.h"
#include <engine/config.h>
#include <engine/server.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>
#include <game/server/teams.h>

#include "character.h"

const float PLASMA_ACCEL = 1.1f;

CPlasma::CPlasma(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, bool Freeze,
	bool Explosive) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Pos = Pos;
	m_Core = Dir;
	m_Freeze = Freeze;
	m_Explosive = Explosive;
	m_EvalTick = Server()->Tick();
	m_LifeTime = Server()->TickSpeed() * 1.5f;
	GameWorld()->InsertEntity(this);
}

bool CPlasma::HitCharacter()
{
	vec2 To2;
	CCharacter *Hit = GameWorld()->IntersectCharacter(m_Pos,
		m_Pos + m_Core, 0.0f, To2);
	if(!Hit)
		return false;

	m_Freeze ? Hit->Freeze() : Hit->UnFreeze();
	if(m_Explosive)
		GameWorld()->CreateExplosion(m_Pos, -1, WEAPON_GRENADE, 0, true);
	GameWorld()->DestroyEntity(this);
	return true;
}

void CPlasma::Move()
{
	m_Pos += m_Core;
	m_Core *= PLASMA_ACCEL;
}

void CPlasma::Reset()
{
	GameWorld()->DestroyEntity(this);
}

void CPlasma::Tick()
{
	if(m_LifeTime == 0)
	{
		Reset();
		return;
	}
	m_LifeTime--;
	Move();
	HitCharacter();

	int Res = 0;
	Res = GameServer()->Collision()->IntersectNoLaser(m_Pos, m_Pos + m_Core, 0,
		0);
	if(Res)
	{
		if(m_Explosive)
			GameWorld()->CreateExplosion(
				m_Pos,
				-1,
				WEAPON_GRENADE,
				0,
				true,
				-1LL);
		Reset();
	}
}

void CPlasma::Snap(int SnappingClient, bool IsOther)
{
	if(NetworkClipped(SnappingClient))
		return;

	int Tick = (Server()->Tick() % Server()->TickSpeed()) % 11;

	if(m_Layer == LAYER_SWITCH && m_Number > 0 && !GameServer()->Collision()->m_pSwitchers[m_Number].m_Status[GameWorld()->Team()] && (!Tick))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(
		NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));

	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_Pos.x;
	pObj->m_FromY = (int)m_Pos.y;
	pObj->m_StartTick = m_EvalTick;
}
