#include "stdafx.h"
#include "map_hint.h"
#include "../../xrUI/Widgets/UIStatic.h"
#include "../../xrUI/Widgets/UIFrameWindow.h"
#include "../../xrUI/UIXmlInit.h"

CUIMapHint::~CUIMapHint	()
{}

void CUIMapHint::Init		()
{
	CUIXmlInit xml_init;
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "hint_item.xml");
	
	m_border			= new CUIFrameWindow();m_border->SetAutoDelete(true);
	AttachChild			(m_border);
	xml_init.InitFrameWindow(uiXml,"button_hint",0,m_border);

	m_text				= new CUIStatic();m_text->SetAutoDelete(true);
	AttachChild			(m_text);
	xml_init.InitStatic	(uiXml,"button_hint:description",0,m_text);

}

void CUIMapHint::SetText		(LPCSTR text)
{
	m_text->SetTextST			(text);
	m_text->AdjustHeightToText	();
	float new_h					= _max(64.0f, m_text->GetWndPos().y+m_text->GetWndSize().y+20.0f);
	SetWndSize					(Fvector2().set(GetWndSize().x, new_h));
	m_border->SetWidth			(GetWndSize().x);
	m_border->SetHeight			(GetWndSize().y);
}
void CUIMapHint::Draw_		()
{
	inherited::Draw			();
}
