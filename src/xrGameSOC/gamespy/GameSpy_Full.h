#pragma once

class CGameSpy_Available;
class CGameSpy_HTTP;
class CGameSpy_Browser;

#include "GameSpy_FuncDefs.h"
//#include "../../xrGameSpy/GameSpy/common/gsCore.h"
typedef unsigned int gsi_time;  // must be 32 bits - sorry for copying :(

class CGameSpy_Full
{
	void	LoadGameSpy();

	bool	m_bServicesAlreadyChecked;
public:
	CGameSpy_Full	();
	~CGameSpy_Full	();

	CGameSpy_Available*	GetGameSpyAvailable	() const { return m_pGSA; };
	CGameSpy_HTTP*		GetGameSpyHTTP		() const { return m_pGS_HTTP; };
	CGameSpy_Browser*	GetGameSpyBrowser	() const { return m_pGS_SB; };

	void		Update			();
	const char*	GetGameVersion	();
	
	void		CoreThink		(gsi_time theMs)	{ xrGS_gsCoreThink(theMs);	};
private:

	CGameSpy_Available*	m_pGSA;
	CGameSpy_HTTP*		m_pGS_HTTP;
	CGameSpy_Browser*	m_pGS_SB;

	void		CoreInitialize	()					{ xrGS_gsCoreInitialize();	};
	void		CoreShutdown	()					{ xrGS_gsCoreShutdown();	};
	GAMESPY_FN_VAR_DECL(const char*, GetGameVersion,());
	
	GAMESPY_FN_VAR_DECL(void,	gsCoreInitialize,	());
	GAMESPY_FN_VAR_DECL(void,	gsCoreThink,		(gsi_time theMs));
	GAMESPY_FN_VAR_DECL(void,	gsCoreShutdown,		());
};
