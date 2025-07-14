// File:		UIBuyWeaponTab.h
// Description:	Tab conorol for BuyWeaponWnd
// Created:		07.02.2005
// Author:		Serge Vynnychenko
// Mail:		narrator@gsc-game.kiev.ua

// Copyright 2005 GSC Game World

#pragma once

#include "../../xrUI/Widgets/UITabControl.h"
#include "../../xrUI/Widgets/UIStatic.h"

class CUIXml;

class CUIBuyWeaponTab : public CUITabControl 
{
public:
				CUIBuyWeaponTab				();
	virtual		~CUIBuyWeaponTab			();

	virtual void Init						(CUIXml* xml, char* path);
 	virtual void OnTabChange				(const shared_str& sCur, const shared_str& sPrev);
			void SetActiveState				(bool bState = true);
private:
	bool		m_bActiveState;
	shared_str	m_sStubId;
};
