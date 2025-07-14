#pragma once
#include "../../xrUI/Widgets/UIDialogWnd.h"
#include "../../xrUI/Widgets/UIWndCallback.h"
//#include "UIMessageBox.h"

class CUIMessageBox;

class CUIMessageBoxEx : public CUIDialogWnd, public CUIWndCallback{
public:
	CUIMessageBoxEx();
	virtual ~CUIMessageBoxEx();
			void SetText(LPCSTR text);
			LPCSTR GetText ();
	virtual void Init(LPCSTR xml_template);
	virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = nullptr);

	LPCSTR		 GetHost			();
	LPCSTR		 GetPassword		();

    CUIMessageBox*	m_pMessageBox;
	virtual bool	OnKeyboardAction			(int dik, EUIMessages keyboard_action);
	virtual bool	NeedCenterCursor	()const	 {return false;}
};