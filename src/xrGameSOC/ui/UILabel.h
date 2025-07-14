#pragma once

#include "../xrUI/Widgets/UIFrameLineWnd.h"
#include "../xrUI/Widgets/UILines.h"
#include "../xrUI/Widgets/UIStatic.h"
#include "../../xrScripts/script_export_space.h"

class CLAItem;

class CUILabel : public CUIFrameLineWnd
{
	CLAItem*				m_lanim;
	float					m_lainm_start_time;

public:
	//IUISimpleWindow
	virtual void SetWidth	(float width);
	virtual void SetHeight	(float height);

    // CUIFrameLineWnd
			void			InitLabel		(Fvector2 pos, Fvector2 size);
	virtual void			Draw			();
	virtual void			Update			();

	virtual void			SetText					(LPCSTR txt)				{m_text.TextItemControl()->SetText(txt);}
	virtual LPCSTR			GetText					()							{return m_text.TextItemControl()->GetText();}

	virtual void			SetTextColor					(u32 color)				{m_text.TextItemControl()->SetTextColor(color);}
	virtual u32			GetTextColor				()							{return m_text.TextItemControl()->GetTextColor();}

	// own
	CUILabel();
			void			SetLightAnim			(LPCSTR lanim);
	CUIStatic				m_text;
	DECLARE_SCRIPT_REGISTER_FUNCTION
protected:
	Fvector2	m_textPos;
};