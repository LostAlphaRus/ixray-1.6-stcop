#pragma once

#include "../../xrUI/Widgets/UIScrollView.h"
#include "UIStatsPlayerList.h"

class CUIXml;
class CUIFrameWindow;

class CUIStats : public CUIScrollView {
public:

	CUIStats();
	virtual ~CUIStats();
	CUIWindow* Init(CUIXml& xml_doc, LPCSTR path,int team);
};