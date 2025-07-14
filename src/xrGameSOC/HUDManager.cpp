#include "stdafx.h"
#include "HUDManager.h"
#include "hudtarget.h"
#include "UIGameCustom.h"

#include "actor.h"
#include "../xrEngine/igame_level.h"
#include "clsid_game.h"
#include "GamePersistent.h"
#include "UIFontDefines.h"
#include "game_cl_base.h"
#include "../xrUI/ui_base.h"

extern CUIGameCustom* CurrentGameUI() { return HUD().GetGameUI(); }

//--------------------------------------------------------------------
CHUDManager::CHUDManager()
{ 
	pUIGame					= nullptr;
	m_pHUDTarget			= new CHUDTarget();
}
//--------------------------------------------------------------------
CHUDManager::~CHUDManager()
{
	OnDisconnected			();

	if (pUIGame)
		pUIGame->UnLoad();

	xr_delete			(pUIGame);
	xr_delete			(m_pHUDTarget);
}

//--------------------------------------------------------------------

void CHUDManager::Load()
{
	if (!pUIGame)
	{
		pUIGame = Game().createGameUI();
	}
	else
	{
		pUIGame->SetClGame(&Game());
	}
	OnDisconnected		();
}
//--------------------------------------------------------------------
void CHUDManager::OnFrame()
{
	if(!b_online)					
		return;

	if (pUIGame)
		pUIGame->OnFrame();

	PROF_EVENT("CHUDManager::OnFrame");
	m_pHUDTarget->CursorOnFrame();
}
//--------------------------------------------------------------------

ENGINE_API extern float psHUD_FOV;

void CHUDManager::Render_First()
{
	if (!psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT))
		return;

	if (!pUIGame)					
		return;

	CObject*	O					= g_pGameLevel->CurrentViewEntity();
	if (0==O)						return;
	CActor*		A					= smart_cast<CActor*> (O);
	if (!A)							return;
	if (A && !A->HUDview())			return;

	// only shadow 
	::Render->set_Invisible			(TRUE);
	::Render->set_Object			(O->H_Root());
	O->renderable_Render			();
	::Render->set_Invisible			(FALSE);
}

void CHUDManager::Render_Last()
{
	if (!psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT))
		return;

	if (!pUIGame)					
		return;

	CObject*	O					= g_pGameLevel->CurrentViewEntity();
	if (0==O)						return;
	CActor*		A					= smart_cast<CActor*> (O);
	if (A && !A->HUDview())			return;
	if(O->CLS_ID == CLSID_CAR)
		return;

	if(O->CLS_ID == CLSID_SPECTATOR)
		return;

	// hud itself
	::Render->set_HUD				(TRUE);
	::Render->set_Object			(O->H_Root());
	O->OnHUDDraw					(this);
	::Render->set_HUD				(FALSE);
}
extern void draw_wnds_rects();
extern ENGINE_API BOOL bShowPauseString;
//отрисовка элементов интерфейса
#include "../xrEngine/string_table.h"

xrCriticalSection ui_lock;
void  CHUDManager::RenderUI()
{
	if(!b_online)					return;

	if (true)
	{
		HitMarker.Render			();
		if (pUIGame)
		{
			xrCriticalSectionGuard guard(&ui_lock);
			pUIGame->Render();
		}
		UI().Font().Render();
	}

	if (psHUD_Flags.is(HUD_CROSSHAIR|HUD_CROSSHAIR_RT|HUD_CROSSHAIR_RT2))	
		m_pHUDTarget->Render();

	draw_wnds_rects		();

	if( Device.Paused() && bShowPauseString){
		CGameFont* pFont	= UI().Font().GetFont(GRAFFITI50_FONT_NAME);
		pFont->SetColor		(0x80FF0000	);
		LPCSTR _str			= g_pStringTable->translate("st_game_paused").c_str();
		
		Fvector2			_pos;
		_pos.set			(UI_BASE_WIDTH/2.0f, UI_BASE_HEIGHT/2.0f);
		UI().ClientToScreenScaled(_pos);
		pFont->SetAligment	(CGameFont::alCenter);
		pFont->Out			(_pos.x, _pos.y, _str);
		pFont->OnRender		();
	}

}

void CHUDManager::OnEvent(EVENT E, u64 P1, u64 P2)
{
}

collide::rq_result&	CHUDManager::GetCurrentRayQuery	() 
{
	return m_pHUDTarget->RQ;
}

void CHUDManager::SetCrosshairDisp	(float dispf, float disps)
{	
	m_pHUDTarget->HUDCrosshair.SetDispersion(psHUD_Flags.test(HUD_CROSSHAIR_DYNAMIC) ? dispf : disps);
}

void  CHUDManager::ShowCrosshair	(bool show)
{
	m_pHUDTarget->m_bShowCrosshair = show;
}


void CHUDManager::Hit(int idx, float power, const Fvector& dir)	
{
	HitMarker.Hit(idx, dir);
}

void CHUDManager::SetHitmarkType		(LPCSTR tex_name)
{
	HitMarker.InitShader				(tex_name);
}
#include "ui\UIMainInGameWnd.h"
#include "UIGameCustom.h"
extern CUIXml* pWpnScopeXml;

void CHUDManager::OnScreenResolutionChanged()
{
	pUIGame->HideShownDialogs();

	if (EngineExternal()[EEngineExternalGame::UseNewScopeSystem])
		xr_delete							(pWpnScopeXml);

	pUIGame->UnLoad							();
	pUIGame->Load							();
	pUIGame->OnConnected					();

	if (pUIGame)
		Game().OnScreenResolutionChanged();
}

void CHUDManager::OnDisconnected()
{
	b_online				= false;
}

void CHUDManager::OnConnected()
{
	if(b_online)			return;
	b_online				= true;
}

void CHUDManager::net_Relcase	(CObject *object)
{
	VERIFY						(m_pHUDTarget);
	m_pHUDTarget->net_Relcase	(object);
}
