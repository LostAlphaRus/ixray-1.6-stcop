#pragma once


#include "../../xrUI/Widgets/UIStatic.h"
#include "../../xrUI/Widgets/UI3tButton.h"
#include "../../xrUI/Widgets/UIFrameLineWnd.h"

#include "../InfoPortion.h"

#include "UICharacterInfo.h"
#include "UIItemInfo.h"

#include "../../xrUI/Widgets/UIWndCallback.h"

class CUIScrollView;
class CUIXml;

class CUITalkDialogWnd: public CUIWindow, public CUIWndCallback
{
private:
	typedef CUIWindow inherited;
	CUIXml*			m_uiXml;
public:
	CUITalkDialogWnd();
	virtual ~CUITalkDialogWnd();
	

	virtual void InitTalkDialogWnd();
	
	virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual void Show(bool status);

	u32			GetHeaderColor()		{ return m_iNameTextColor; }
	CGameFont *	GetHeaderFont()			{ return m_pNameTextFont; }
	u32			GetOurReplicsColor()	{ return m_uOurReplicsColor; }

	shared_str			m_ClickedQuestionID;

	//список вопросов, которые мы можем задавать персонажу

	//элементы интерфейса диалога
	CUIStatic*			UIDialogFrameTop;
	CUIStatic*			UIDialogFrameBottom;
	CUIFrameLineWnd		UIDialogFrame;
	CUIFrameLineWnd		UIOurPhrasesFrame;

	CUIStatic			UIStaticTop;
	CUIStatic			UIStaticBottom;

	CUI3tButton			UIToTradeButton;

	//информация о персонажах 
	CUIStatic			UIOurIcon;
	CUIStatic			UIOthersIcon;
	CUICharacterInfo	UICharacterInfoLeft;
	CUICharacterInfo	UICharacterInfoRight;

	void				AddQuestion			(LPCSTR str, LPCSTR value);
	void				AddAnswer			(LPCSTR SpeakerName, const char* str, bool bActor);
	void				AddIconedAnswer		(LPCSTR text, LPCSTR texture_name, Frect texture_rect, LPCSTR templ_name);
	void				ClearAll			();
	void				ClearQuestions		();

	void				SetOsoznanieMode	(bool b);
private:
	CUIScrollView*			UIQuestionsList;
	CUIScrollView*			UIAnswersList;

	// Шрифт и цвет текста с именем персонажа
	CGameFont			*m_pNameTextFont;
	u32					m_iNameTextColor;
	// Цвет тeкста и шрифт наших реплик
	u32					m_uOurReplicsColor;

	void 		OnTradeClicked			(CUIWindow* w, void*);
	void 		OnQuestionClicked		(CUIWindow* w, void*);
	
};


class CUIQuestionItem :public CUIWindow, public CUIWndCallback
{
	typedef CUIWindow inherited;
	float			m_min_height;
public:
	CUI3tButton*	m_text;
	shared_str		m_s_value;
					CUIQuestionItem			(CUIXml* xml_doc, LPCSTR path);
	void			Init					(LPCSTR val, LPCSTR text);

	virtual void	SendMessage				(CUIWindow* pWnd, s16 msg, void* pData = NULL);
	void 	OnTextClicked			(CUIWindow* w, void*);
};

class CUIAnswerItem :public CUIWindow
{
	typedef CUIWindow inherited;

	float			m_min_height;
	float			m_bottom_footer;
	CUIStatic*		m_text;
	CUIStatic*		m_name;
public:
					CUIAnswerItem			(CUIXml* xml_doc, LPCSTR path);
	void			Init					(LPCSTR text, LPCSTR name);
};

class CUIAnswerItemIconed :public CUIAnswerItem
{
	typedef CUIAnswerItem inherited;
	CUIStatic*		m_icon;

public:
					CUIAnswerItemIconed		(CUIXml* xml_doc, LPCSTR path);
	void			Init					(LPCSTR text, LPCSTR texture_name, Frect texture_rect);

};
