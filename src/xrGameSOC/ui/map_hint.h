#pragma once
#include "../../xrUI/Widgets/UIWindow.h"

class CUIStatic;
class CUIFrameWindow;

class CUIMapHint :public CUIWindow
{
	typedef CUIWindow inherited;
	CUIStatic*			m_text;
	CUIFrameWindow*		m_border;
	CUIWindow*			m_owner;
public:
					CUIMapHint	():m_owner(nullptr){};
	virtual 		~CUIMapHint	();
	void			Init		();
	void			SetText		(LPCSTR text);
			void	Draw		();
	void			SetOwner	(CUIWindow* w)	{m_owner = w;}
	CUIWindow*		GetOwner	()				{return m_owner;}
	bool			m_visible;
};