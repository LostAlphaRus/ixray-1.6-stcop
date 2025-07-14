#include "stdafx.h"
#include "UIGameTDM.h"

//. #include "UITDMPlayerList.h"
//.#include "UITDMFragList.h"
#include "UIDMStatisticWnd.h"

#include "hudmanager.h"
#include "game_cl_base.h"

#include "game_cl_TeamDeathmatch.h"

#include "ui/UIFrags2.h"
#include "ui/TeamInfo.h"

#include "../xrCore/object_broker.h"

#define MSGS_OFFS 510

//--------------------------------------------------------------------
CUIGameTDM::CUIGameTDM()
{
	m_game							= nullptr;

	CUIXml							uiXml;
	CUIXmlInit						xml_init;
	uiXml.Load						(CONFIG_PATH, UI_PATH, "ui_game_tdm.xml");
	m_team1_icon					= new CUIStatic();
	m_team2_icon					= new CUIStatic();
	m_team1_score					= new CUIStatic();
	m_team2_score					= new CUIStatic();
	xml_init.InitStatic				(uiXml, "team1_icon", 0,	m_team1_icon);
//.	xml_init.InitAutoStaticGroup	(uiXml, "team1_icon",		m_team1_icon);
	xml_init.InitStatic				(uiXml, "team2_icon", 0,	m_team2_icon);
//.	xml_init.InitAutoStaticGroup	(uiXml, "team2_icon",		m_team2_icon); moved in InitStatic

//.	CUIXmlInit::StaticsVec gr;
	xml_init.InitStatic					(uiXml, "team1_score", 0,	m_team1_score);
//.	gr = xml_init.InitAutoStaticGroup	(uiXml, "team1_score",		m_team1_score);
//.	(*gr.back()).SetText				(*CTeamInfo::GetTeam1_name());
//.	(*gr.back()).SetTextColor			(CTeamInfo::GetTeam1_color());

	xml_init.InitStatic					(uiXml, "team2_score", 0,	m_team2_score);
//.	gr = xml_init.InitAutoStaticGroup	(uiXml, "team2_score",		m_team2_score);
//.	(*gr.back()).SetText				(*CTeamInfo::GetTeam2_name());
//.	(*gr.back()).SetTextColor			(CTeamInfo::GetTeam2_color());

	m_pUITeamSelectWnd	= new CUISpawnWnd	();
}
//--------------------------------------------------------------------
void CUIGameTDM::SetClGame (game_cl_GameState* g)
{
	inherited::SetClGame(g);
	m_game = smart_cast<game_cl_TeamDeathmatch*>(g);
	R_ASSERT(m_game);
}

void CUIGameTDM::Init ()
{
	CUIXml xml_doc;
	xml_doc.Load(CONFIG_PATH, UI_PATH, "stats.xml");

	CUIFrags2* pFragList		= new CUIFrags2();			pFragList->SetAutoDelete(true);
	//-----------------------------------------------------------
	CUIDMStatisticWnd* pStatisticWnd = new CUIDMStatisticWnd(); pStatisticWnd->SetAutoDelete(true);

	pFragList->Init(xml_doc,"stats_wnd","frag_wnd_tdm");

	float ScreenW = UI_BASE_WIDTH;
	float ScreenH = UI_BASE_HEIGHT;
	//-----------------------------------------------------------
	Frect FrameRect = pFragList->GetWndRect ();
	float FrameW	= FrameRect.right - FrameRect.left;
	float FrameH	= FrameRect.bottom - FrameRect.top;

	pFragList->SetWndPos(Fvector2().set((ScreenW-FrameW)/2.0f, (ScreenH - FrameH)/2.0f));
	//-----------------------------------------------------------
	m_pFragLists->AttachChild(pFragList);
	//-----------------------------------------------------------
	CUIFrags2* pPlayerListT1	= new CUIFrags2	();pPlayerListT1->SetAutoDelete(true);
//	CUIFrags2* pPlayerListT2	= new CUIFrags2	();pPlayerListT2->SetAutoDelete(true);

	pPlayerListT1->Init(xml_doc, "players_wnd", "frag_wnd_tdm");
//	pPlayerListT2->Init(xml_doc, "players_wnd", "frag_wnd_tdm");
	//-----------------------------------------------------------
	FrameRect = pPlayerListT1->GetWndRect ();
	FrameW	= FrameRect.right - FrameRect.left;
	FrameH	= FrameRect.bottom - FrameRect.top;

	pPlayerListT1->SetWndPos(Fvector2().set((ScreenW-FrameW)/2.0f, (ScreenH - FrameH)/2.0f));
	//-----------------------------------------------------------
//	FrameRect = pPlayerListT2->GetWndRect ();
//	FrameW	= FrameRect.right - FrameRect.left;
//	FrameH	= FrameRect.bottom - FrameRect.top;

//	pPlayerListT2->SetWndRect(ScreenW/4.0f*3.0f-FrameW/2.0f, (ScreenH - FrameH)/2.0f, FrameW, FrameH);
	//-----------------------------------------------------------
	m_pPlayerLists->AttachChild(pPlayerListT1);
//	m_pPlayerLists->AttachChild(pPlayerListT2);
	//-----------------------------------------------------------
	FrameRect = pStatisticWnd->GetFrameRect ();
	FrameW	= FrameRect.right - FrameRect.left;
	FrameH	= FrameRect.bottom - FrameRect.top;
	pStatisticWnd->SetWndRect(Frect().set((ScreenW-FrameW)/2.0f, (ScreenH - FrameH)/2.0f, FrameW, FrameH));

	m_pStatisticWnds->AttachChild(pStatisticWnd);
}
//--------------------------------------------------------------------
CUIGameTDM::~CUIGameTDM()
{
	xr_delete			(m_team1_icon);
	xr_delete			(m_team2_icon);
	xr_delete			(m_team1_score);
	xr_delete			(m_team2_score);

	delete_data			(m_pUITeamSelectWnd);
}
//--------------------------------------------------------------------
bool CUIGameTDM::IR_UIOnKeyboardPress(int dik)
{
	switch (dik) {
		case SDL_SCANCODE_CAPSLOCK :
		{
			if (m_game)
			{
				if (m_game->Get_ShowPlayerNamesEnabled())
					m_game->Set_ShowPlayerNames( !m_game->Get_ShowPlayerNames() );
				else
					m_game->Set_ShowPlayerNames(true);
				return true;
			};
		}break;
	}
	if(inherited::IR_UIOnKeyboardPress(dik)) return true;
	return false;
}

bool CUIGameTDM::IR_UIOnKeyboardRelease(int dik)
{
	switch (dik) {
		case SDL_SCANCODE_CAPSLOCK :
			{
				if (m_game)
				{
					if (!m_game->Get_ShowPlayerNamesEnabled())						
						m_game->Set_ShowPlayerNames(false);
					return true;
				};
			}break;
	}
	if(inherited::IR_UIOnKeyboardRelease(dik)) return true;
	
	return false;
}

void CUIGameTDM::OnFrame()
{
	inherited::OnFrame();
	m_team1_icon->Update();
	m_team2_icon->Update();
	m_team1_score->Update();
	m_team2_score->Update();
}

void CUIGameTDM::Render()
{
	inherited::Render();
	m_team1_icon->Draw();
	m_team2_icon->Draw();
	m_team1_score->Draw();
	m_team2_score->Draw();
}

void CUIGameTDM::SetScoreCaption(int t1, int t2)
{
	string32				str;
	sprintf_s					(str,"%d", t1);
	m_team1_score->SetText	(str);

	sprintf_s					(str,"%d", t2);
	m_team2_score->SetText	(str);
}

void CUIGameTDM::SetFraglimit(int local_frags, int fraglimit)
{
	string64 str;
	if(fraglimit)
		sprintf_s(str,"%d", fraglimit);
	else
		sprintf_s(str,"%s", "--");

	m_pFragLimitIndicator->SetText(str);
}
