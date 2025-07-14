#pragma once

#include "../../xrScripts/script_callback_ex.h"

class CUIWindow;
struct SCallbackInfo{
	CScriptCallbackEx<void>	m_callback;
	xr_delegate<void(CUIWindow*,void*)>				m_cpp_callback;
	shared_str				m_controlName;
	s16						m_event;
	SCallbackInfo():m_controlName(""),m_event(-1){};
};

