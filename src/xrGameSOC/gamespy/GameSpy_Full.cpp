#include "StdAfx.h"
#include "GameSpy_Full.h"

#include "GameSpy_Available.h"
#include "GameSpy_HTTP.h"
#include "GameSpy_Browser.h"
#include "../MainMenu.h"
#include "../xrCore/object_broker.h"


CGameSpy_Full::CGameSpy_Full()	
{
	m_pGSA	= nullptr;
	m_pGS_HTTP = nullptr;
	m_pGS_SB = nullptr;

	m_bServicesAlreadyChecked	= false;

	if (Engine.External.hGameSpy == 0)
		return;

	LoadGameSpy();
	//---------------------------------------
	m_pGSA = new CGameSpy_Available();
	//-----------------------------------------------------
	shared_str resultstr;
	m_bServicesAlreadyChecked = m_pGSA->CheckAvailableServices(resultstr);
	//-----------------------------------------------------
	if (Engine.External.hGameSpy != 0)
	{
		CoreInitialize();
		m_pGS_HTTP = new CGameSpy_HTTP();
		m_pGS_SB = new CGameSpy_Browser();
	}
}

CGameSpy_Full::~CGameSpy_Full()
{
	if (Engine.External.hGameSpy != 0)
	{
		delete_data(m_pGSA);
		delete_data(m_pGS_HTTP);
		delete_data(m_pGS_SB);

		CoreShutdown();
	}
}

void CGameSpy_Full::LoadGameSpy()
{
	GAMESPY_LOAD_FN			(xrGS_GetGameVersion);
	GAMESPY_LOAD_FN			(xrGS_gsCoreInitialize);
	GAMESPY_LOAD_FN			(xrGS_gsCoreThink);
	GAMESPY_LOAD_FN			(xrGS_gsCoreShutdown);
}

void CGameSpy_Full::Update()
{
	if (Engine.External.hGameSpy == 0)
		return;

	if (!m_bServicesAlreadyChecked)
	{
		m_bServicesAlreadyChecked = true;
		MainMenu()->SetErrorDialog(CMainMenu::ErrGSServiceFailed);
	}
	m_pGS_HTTP->Think	();
	m_pGS_SB->Update	();
	CoreThink			(15);
};

const char* CGameSpy_Full::GetGameVersion(int PlatformID)
{
	return xrGS_GetGameVersion(PlatformID);
};