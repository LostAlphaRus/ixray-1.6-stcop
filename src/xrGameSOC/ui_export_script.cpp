#include "pch_script.h"

#include "script_ui_registrator.h"
#include "UI\UIMultiTextStatic.h"
#include "MainMenu.h"

#include "uiwindow.h"
#include "uimmshniaga.h"
#include "uistatic.h"
#include "uibutton.h"
#include "UIProgressBar.h"
#include "UIComboBox.h"
#include "UITabControl.h"
#include "UIMessageBox.h"
#include "uilistwnd.h"
#include "UIMapList.h"
#include "UIWindow.h"
#include "UIScriptWnd.h"
#include "UIGame_custom_script.h"
#include "../ScriptXMLInit.h"
#include "UIPropertiesbox.h"
#include "UIOptionsManagerScript.h"
#include "UIMapInfo.h"


using namespace luabind;

CMainMenu*	MainMenu();

#pragma optimize("s",on)
void UIRegistrator::script_register(lua_State *L)
{
	CUIWindow::script_register(L);
	CUIMMShniaga::script_register(L);
	CUIStatic::script_register(L);
	CUIButton::script_register(L);
	CUIProgressBar::script_register(L);
	CUIComboBox::script_register(L);
	CUIEditBox::script_register(L);
	CUITabControl::script_register(L);
	CUIMessageBox::script_register(L);
	CUIListWnd::script_register(L);
	CUIMapList::script_register(L);
	CUIDialogWndEx::script_register(L);
	CUIPropertiesBox::script_register(L);
	CUIOptionsManagerScript::script_register(L);
	CUIMapInfo::script_register(L);
	CScriptXmlInit::script_register(L);
	CUIGameCustom::script_register(L);
	UIGame_custom_script::script_register(L);

	module(L)
	[

		class_<CGameFont>("CGameFont")
			.enum_("EAligment")
			[
				value("alLeft",						int(CGameFont::alLeft)),
				value("alRight",					int(CGameFont::alRight)),
				value("alCenter",					int(CGameFont::alCenter))
			],

		class_<CUICaption>("CUICaption")
			.def("addCustomMessage",	&CUICaption::addCustomMessage)
			.def("setCaption",			&CUICaption::setCaption),

		class_<Patch_Dawnload_Progress>("Patch_Dawnload_Progress")
			.def("GetInProgress",	&Patch_Dawnload_Progress::GetInProgress)
			.def("GetStatus",		&Patch_Dawnload_Progress::GetStatus)
			.def("GetFlieName",		&Patch_Dawnload_Progress::GetFlieName)
			.def("GetProgress",		&Patch_Dawnload_Progress::GetProgress),

		class_<CMainMenu>("CMainMenu")
			.def("GetPatchProgress",		&CMainMenu::GetPatchProgress)
			.def("CancelDownload",			&CMainMenu::CancelDownload)
			.def("ValidateCDKey",			&CMainMenu::ValidateCDKey)
			.def("GetGSVer",				&CMainMenu::GetGSVer)
	],
	module(L,"main_menu")
	[
		def("get_main_menu",				&MainMenu)
	];

}
