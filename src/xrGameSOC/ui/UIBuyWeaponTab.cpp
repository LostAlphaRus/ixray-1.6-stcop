// File:		UIBuyWeaponTab.cpp
// Description:	Tab conorol for BuyWeaponWnd
// Created:		07.02.2005
// Author:		Serge Vynnychenko
// Mail:		narrator@gsc-game.kiev.ua

// Copyright 2005 GSC Game World

#include "StdAfx.h"
#include "UIBuyWeaponTab.h"
#include "../../xrUI/Widgets/UITabButtonMP.h"
#include "../../xrEngine/string_table.h"
#include "../HUDManager.h"
#include "../../xrUI/UIXmlInit.h"
#include "../../xrUI/Widgets/UITabControl.h"

CUIBuyWeaponTab::CUIBuyWeaponTab()
{
	m_bActiveState		= true;
}

CUIBuyWeaponTab::~CUIBuyWeaponTab()
{}

void CUIBuyWeaponTab::Init(CUIXml* xml, char* path)
{
 
 	R_ASSERT3					(xml->NavigateToNode(path,0), "XML node not found", path);
 	
 	CUIXmlInit::InitWindow		(*xml, path, 0, this);
 	int tabsCount				= xml->GetNodesNum(path, 0, "button");
 
 	XML_NODE* tab_node			= xml->NavigateToNode(path,0);
 	xml->SetLocalRoot			(tab_node);
 
 	for (int i = 0; i < tabsCount; ++i)
 	{
 		CUITabButtonMP *newButton	= new CUITabButtonMP();
 		CUIXmlInit::Init3tButton	(*xml, "button", i, newButton);
 		newButton->m_btn_id			= xml->ReadAttrib("button",i,"id");
 		R_ASSERT					(newButton->m_btn_id.size());
 		AddItem						(newButton);
 	}
 
 	CUITabButtonMP *stubButton		= new CUITabButtonMP();
 	stubButton->m_btn_id			= "stub";
 	AddItem							(stubButton);
 	m_sStubId						= stubButton->m_btn_id;
 
 	SetActiveTab					(m_sStubId);
 	
 	xml->SetLocalRoot				(xml->GetRoot());
 
 	SetActiveState					();
}

void CUIBuyWeaponTab::OnTabChange(const shared_str& iCur, const shared_str& iPrev)
{
	CUITabControl::OnTabChange		(iCur, iPrev);

	if (m_sStubId != iCur)
        SetActiveState				(false);	
}

void CUIBuyWeaponTab::SetActiveState(bool bState)
{
 	m_bActiveState					= bState;
 
 	WINDOW_LIST::iterator it		= m_ChildWndList.begin();
 
 	for(u32 i=0; i<m_ChildWndList.size(); ++i, ++it)
 		(*it)->Enable(bState);
 
 	if (bState)
 		SetActiveTab				(m_sStubId);
 
 	GetButtonById(m_sPushedId)->Enable(true);
}
