#include "pch_script.h"
#include "UILabel.h"

using namespace luabind;

#pragma optimize("s",on)
void CUILabel::script_register(lua_State *L){
	module(L)
	[
		class_<CUILabel, CUIFrameLineWnd>("CUILabel")
		.def(				constructor<>())
		.def("SetText",		&CUILabel::SetText)
		.def("GetText",		&CUILabel::GetText)
	];

}