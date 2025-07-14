#pragma once

#include "../../xrUI/Widgets/UIDialogWnd.h"

class CUIStatic;
class CUI3tButton;
class CUIEditBox;
class CUIXml;

class CUITextVote : public CUIDialogWnd 
{
public:

					CUITextVote		();

			void	Init			(CUIXml& xml_doc);

	virtual void	SendMessage		(CUIWindow* pWnd, s16 msg, void* pData = 0);

	void 			OnBtnOk			();
	void 			OnBtnCancel		();

protected:
	CUIStatic*		bkgrnd;
	CUIStatic*		header;
	CUIEditBox*	edit;

	CUI3tButton*	btn_ok;
	CUI3tButton*	btn_cancel;
};