//=============================================================================
//  Filename:   UIGameLog.h
//	Created by Vitaly 'Mad Max' Maximov, mad-max@gsc-game.kiev.ua
//	Copyright 2005. GSC Game World
//	---------------------------------------------------------------------------
//  Multiplayer game log window
//=============================================================================
#include "stdafx.h"
#include "UIGameLog.h"
#include "../../xrUI/UIXmlInit.h"
#include "UIColorAnimatorWrapper.h"
#include "UIPdaMsgListItem.h"
#include "UIPdaKillMessage.h"
#include "../../xrUI/Widgets/UILines.h"

const char * const	CHAT_LOG_ITEMS_ANIMATION	= "ui_main_msgs_short";

CUIGameLog::CUIGameLog()
{
	toDelList.reserve				(30);
	kill_msg_height					= 20;
	txt_color						= 0xff000000;
}

CUIGameLog::~CUIGameLog()
{}


CUITextWnd* CUIGameLog::AddLogMessage(LPCSTR msg)
{
	CUITextWnd* pItem				= nullptr;
	ADD_TEXT_TO_VIEW3				(msg, pItem, this);
	pItem->SetTextComplexMode		(true);
	pItem->SetFont					(GetFont());
	pItem->SetTextColor				(txt_color);
	pItem->SetColorAnimation		(CHAT_LOG_ITEMS_ANIMATION, LA_ONLYALPHA | LA_TEXTCOLOR, 5000.0f);
	ForceUpdate						();
	return							pItem;
}

// warning: initialization of item is incomplete!
// initialization of item's height, text static and icon still necessary
CUIPdaMsgListItem* CUIGameLog::AddPdaMessage(LPCSTR msg, float delay){
	CUIPdaMsgListItem* pItem				= new CUIPdaMsgListItem();
	pItem->Init								(0,0, GetDesiredChildWidth(), 10);	//fake height
	pItem->UIMsgText.SetTextST				(msg);
	pItem->SetColorAnimation				(CHAT_LOG_ITEMS_ANIMATION, LA_ONLYALPHA | LA_TEXTCOLOR, 5000.0f);
	AddWindow								(pItem, true);

	return pItem;
}

u32 CUIGameLog::GetTextColor(){
	return txt_color;
}

CUIPdaKillMessage* CUIGameLog::AddLogMessage(KillMessageStruct& msg){
	CUIPdaKillMessage* pItem = pItem = new CUIPdaKillMessage();	
	pItem->SetFont(GetFont());
	pItem->SetWidth(GetDesiredChildWidth());
	pItem->SetHeight(kill_msg_height);
	pItem->Init(msg);
	pItem->SetColorAnimation(CHAT_LOG_ITEMS_ANIMATION, LA_ONLYALPHA | LA_TEXTCOLOR, 5000.0f);
	AddWindow(pItem, true);
	return pItem;
}

void CUIGameLog::AddChatMessage(LPCSTR msg, LPCSTR author){
	string256 fullLine;
	sprintf_s(fullLine, "%s %s", author, msg);
	_TrimRight	(fullLine);
    
	CUITextWnd* pItem = new CUITextWnd();
	pItem->SetTextComplexMode		(true);
	pItem->SetText(fullLine);
    pItem->SetCutWordsMode(true);
	pItem->SetFont(GetFont());
	pItem->SetTextColor(txt_color);
	pItem->SetColorAnimation(CHAT_LOG_ITEMS_ANIMATION, LA_ONLYALPHA | LA_TEXTCOLOR, 5000.0f);
	pItem->SetWidth(this->GetDesiredChildWidth());
	pItem->AdjustHeightToText();
	AddWindow(pItem, true);	
}

void CUIGameLog::SetTextAtrib(CGameFont* pFont, u32 color){
	SetFont(pFont);
	txt_color = color;
}

void CUIGameLog::Update()
{
	CUIScrollView::Update();
	toDelList.clear();	


	// REMOVE ITEMS WITH COMPLETED ANIMATION
	WINDOW_LIST_it end_it = m_pad->GetChildWndList().end();
	WINDOW_LIST_it begin_it = m_pad->GetChildWndList().begin();

	for(WINDOW_LIST_it it = begin_it; it!=end_it; ++it)
	{
		CUIStatic* pItem = smart_cast<CUIStatic*>(*it);
		VERIFY(pItem);
		pItem->Update();

		if (!pItem->IsColorAnimationPresent())
			toDelList.push_back(pItem);
	}

	// Delete elements
	{
		xr_vector<CUIWindow*>::iterator it;
		for (it = toDelList.begin(); it != toDelList.end(); it++)
			RemoveWindow(*it);
	}

	// REMOVE INVISIBLE AND PART VISIBLE ITEMS
	if(m_flags.test	(eNeedRecalc) )
		RecalcSize			();

	toDelList.clear();
	Frect visible_rect;
	GetAbsoluteRect(visible_rect);
	for(	WINDOW_LIST_it it = m_pad->GetChildWndList().begin(); 
			m_pad->GetChildWndList().end()!=it; 
			++it)
	{
		Frect	r;
		(*it)->GetAbsoluteRect(r);
		if(! (visible_rect.in(r.x1, r.y1) && visible_rect.in(r.x2, r.y1) && visible_rect.in(r.x1, r.y2) && visible_rect.in(r.x2, r.y2)))
		{
			toDelList.push_back(*it);			
		}
			
	}

	// Delete elements
	{
		xr_vector<CUIWindow*>::iterator it;
		for (it = toDelList.begin(); it != toDelList.end(); it++)
			RemoveWindow(*it);
	}

	if(m_flags.test	(eNeedRecalc) )
		RecalcSize			();
}