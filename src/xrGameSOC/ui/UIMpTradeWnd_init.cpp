#include "stdafx.h"
#include "UIMpTradeWnd.h"

#include "../../xrUI/UIXmlInit.h"
#include "UIMpItemsStoreWnd.h"
#include "../../xrUI/Widgets/UITabControl.h"
#include "../../xrUI/Widgets/UITabButtonMP.h"
#include "UIDragDropListEx.h"
#include "UIItemInfo.h"
#include "../UIHelperGame.h"

#include "../../xrCore/object_broker.h"

LPCSTR _list_names[]= {
		"lst_pistol",
		"lst_pistol_ammo",
		"lst_rifle",
		"lst_rifle_ammo",
		"lst_outfit",
		"lst_medkit",
		"lst_granade",
		"lst_others",
		"lst_player_bag",
		"lst_shop",
};
CUIMpTradeWnd::CUIMpTradeWnd()
{
	m_money								= 0;
	g_mp_restrictions.InitGroups		();
	m_bIgnoreMoneyAndRank				= false;
}

CUIMpTradeWnd::~CUIMpTradeWnd()
{
	m_root_tab_control->RemoveAll		();
	delete_data							(m_store_hierarchy);
	delete_data							(m_list[e_shop]);
	delete_data							(m_all_items);
	delete_data							(m_item_mngr);
}

void CUIMpTradeWnd::Init(const shared_str& sectionName, const shared_str& sectionPrice)
{
	m_sectionName						= sectionName;
	m_sectionPrice						= sectionPrice;

	CUIXml								xml_doc;
	xml_doc.Load(CONFIG_PATH, UI_PATH, "mp_buy_menu.xml");

	m_store_hierarchy					= new CStoreHierarchy();
 	m_store_hierarchy->Init				(xml_doc, "items_hierarchy");
	m_store_hierarchy->InitItemsInGroup	(m_sectionName);

	CUIXmlInit::InitWindow				(xml_doc, "main",						0, this);

	m_root_tab_control					= new CUITabControl(); AttachChild(m_root_tab_control); m_root_tab_control->SetAutoDelete(true);
	CUIXmlInit::InitTabControl			(xml_doc, "tab_control",				0, m_root_tab_control);
	Register							(m_root_tab_control);
	AddCallbackStr						("tab_control",	TAB_CHANGED,		CUIWndCallback::void_function	(this, &CUIMpTradeWnd::OnRootTabChanged));

	u32 root_cnt						= m_store_hierarchy->GetRoot().ChildCount();
	for(u32 i=0; i<root_cnt; ++i)
	{
		const CStoreHierarchy::item& it	= m_store_hierarchy->GetRoot().ChildAt(i);

		CUITabButtonMP* btn				= it.m_button;	
		m_root_tab_control->AddItem		(btn);
		btn->SetAutoDelete				(false);
	}
	m_root_tab_control->ResetTab		();

	m_shop_wnd							= new CUIWindow();	AttachChild(m_shop_wnd);		m_shop_wnd->SetAutoDelete(true);
	CUIXmlInit::InitWindow				(xml_doc, "shop_wnd",	0, m_shop_wnd);

	m_btn_ok							= new CUI3tButton();AttachChild(m_btn_ok);					m_btn_ok->SetAutoDelete			(true);
	m_btn_cancel						= new CUI3tButton();AttachChild(m_btn_cancel);				m_btn_cancel->SetAutoDelete		(true);
	m_btn_shop_back						= new CUI3tButton();AttachChild(m_btn_shop_back);			m_btn_shop_back->SetAutoDelete	(true);
	m_btns_preset[0]					= new CUI3tButton();AttachChild(m_btns_preset[0]	);		m_btns_preset[0]->SetAutoDelete	(true);
	m_btns_preset[1]					= new CUI3tButton();AttachChild(m_btns_preset[1]	);		m_btns_preset[1]->SetAutoDelete	(true);
	m_btns_preset[2]					= new CUI3tButton();AttachChild(m_btns_preset[2]	);		m_btns_preset[2]->SetAutoDelete	(true);
	m_btns_preset[3]					= new CUI3tButton();AttachChild(m_btns_preset[3]	);		m_btns_preset[3]->SetAutoDelete	(true);
	m_btns_preset[4]					= new CUI3tButton();AttachChild(m_btns_preset[4]	);		m_btns_preset[4]->SetAutoDelete	(true);
	m_btns_save_preset[0]				= new CUI3tButton();AttachChild(m_btns_save_preset[0]);		m_btns_save_preset[0]->SetAutoDelete(true);
	m_btns_save_preset[1]				= new CUI3tButton();AttachChild(m_btns_save_preset[1]);		m_btns_save_preset[1]->SetAutoDelete(true);
	m_btns_save_preset[2]				= new CUI3tButton();AttachChild(m_btns_save_preset[2]);		m_btns_save_preset[2]->SetAutoDelete(true);
	m_btn_reset							= new CUI3tButton();AttachChild(m_btn_reset		);			m_btn_reset->SetAutoDelete		(true);
	m_btn_sell							= new CUI3tButton();AttachChild(m_btn_sell		);			m_btn_sell->SetAutoDelete		(true);

	m_btn_pistol_ammo					= new CUI3tButton();AttachChild(m_btn_pistol_ammo	);		m_btn_pistol_ammo->SetAutoDelete		(true);
	m_btn_pistol_silencer				= new CUI3tButton();AttachChild(m_btn_pistol_silencer);		m_btn_pistol_silencer->SetAutoDelete	(true);
	m_btn_rifle_ammo					= new CUI3tButton();AttachChild(m_btn_rifle_ammo	);		m_btn_rifle_ammo->SetAutoDelete			(true);
	m_btn_rifle_silencer				= new CUI3tButton();AttachChild(m_btn_rifle_silencer);		m_btn_rifle_silencer->SetAutoDelete		(true);
	m_btn_rifle_scope					= new CUI3tButton();AttachChild(m_btn_rifle_scope	);		m_btn_rifle_scope->SetAutoDelete		(true);
	m_btn_rifle_glauncher				= new CUI3tButton();AttachChild(m_btn_rifle_glauncher);		m_btn_rifle_glauncher->SetAutoDelete	(true);
	m_btn_rifle_ammo2					= new CUI3tButton();AttachChild(m_btn_rifle_ammo2	);		m_btn_rifle_ammo2->SetAutoDelete		(true);


	CUIXmlInit::Init3tButton			(xml_doc, "btn_ok",				0, m_btn_ok				);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_cancel",			0, m_btn_cancel			);
	CUIXmlInit::Init3tButton			(xml_doc, "shop_back_btn",		0, m_btn_shop_back		);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_preset_1",		0, m_btns_preset[1]		);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_preset_2",		0, m_btns_preset[2]		);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_preset_3",		0, m_btns_preset[3]		);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_last_set",		0, m_btns_preset[0]		);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_preset_def",		0, m_btns_preset[4]		);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_save_preset_1",	0, m_btns_save_preset[0]);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_save_preset_2",	0, m_btns_save_preset[1]);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_save_preset_3",	0, m_btns_save_preset[2]);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_reset",			0, m_btn_reset			);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_sell",			0, m_btn_sell			);

	CUIXmlInit::Init3tButton			(xml_doc, "btn_pistol_ammo",	0, m_btn_pistol_ammo	);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_pistol_silencer",0, m_btn_pistol_silencer);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_rifle_ammo",		0, m_btn_rifle_ammo		);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_rifle_silencer",	0, m_btn_rifle_silencer	);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_rifle_scope",	0, m_btn_rifle_scope	);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_rifle_glauncher",0, m_btn_rifle_glauncher);
	CUIXmlInit::Init3tButton			(xml_doc, "btn_rifle_ammo2",	0, m_btn_rifle_ammo2	);



	Register							(m_btn_ok				);
	Register							(m_btn_cancel			);
	Register							(m_btn_shop_back		);
	Register							(m_btns_preset[0]		);
	Register							(m_btns_preset[1]		);
	Register							(m_btns_preset[2]		);
	Register							(m_btns_preset[3]		);
	Register							(m_btns_save_preset[0]	);
	Register							(m_btns_save_preset[1]	);
	Register							(m_btns_save_preset[2]	);
	Register							(m_btn_reset			);
	Register							(m_btn_sell				);
	Register							(m_btn_pistol_ammo		);
	Register							(m_btn_pistol_silencer	);
	Register							(m_btn_rifle_ammo		);
	Register							(m_btn_rifle_silencer	);
	Register							(m_btn_rifle_scope		);
	Register							(m_btn_rifle_glauncher	);
	Register							(m_btn_rifle_ammo2		);


	AddCallbackStr						("btn_ok",			BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnOkClicked));
	AddCallbackStr						("btn_cancel",		BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnCancelClicked));
	AddCallbackStr						("btn_shop_back",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnShopBackClicked));
	AddCallbackStr						("sub_btn",			TAB_CHANGED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnSubLevelBtnClicked));
	AddCallbackStr						("sub_btn",			BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnSubLevelBtnClicked));
	AddCallbackStr						("btn_preset_1",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnPreset1Clicked		));
	AddCallbackStr						("btn_preset_2",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnPreset2Clicked		));
	AddCallbackStr						("btn_preset_3",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnPreset3Clicked		));
	AddCallbackStr						("btn_preset_def",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnPresetDefaultClicked	));
	AddCallbackStr						("btn_last_set",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnLastSetClicked		));
	AddCallbackStr						("btn_save_preset_1",BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnSave1PresetClicked	));
	AddCallbackStr						("btn_save_preset_2",BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnSave2PresetClicked	));
	AddCallbackStr						("btn_save_preset_3",BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnSave3PresetClicked	));
	AddCallbackStr						("btn_reset",		BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnResetClicked		));
	AddCallbackStr						("btn_sell",		BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnSellClicked		));

	AddCallbackStr						("btn_pistol_ammo",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnPistolAmmoClicked		));
	AddCallbackStr						("btn_pistol_silencer",	BUTTON_CLICKED,	CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnPistolSilencerClicked	));
	AddCallbackStr						("btn_rifle_ammo",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnRifleAmmoClicked		));
	AddCallbackStr						("btn_rifle_silencer",BUTTON_CLICKED,	CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnRifleSilencerClicked	));
	AddCallbackStr						("btn_rifle_scope",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnRifleScopeClicked		));
	AddCallbackStr						("btn_rifle_glauncher",BUTTON_CLICKED,	CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnRifleGLClicked			));
	AddCallbackStr						("btn_rifle_ammo2",	BUTTON_CLICKED,		CUIWndCallback::void_function	(this,	&CUIMpTradeWnd::OnBtnRifleAmmo2Clicked		));

	for(int idx = e_first; idx<e_total_lists; ++idx)
	{
		CUIDragDropListEx* lst			= new CUIDragDropListEx();
		m_list[idx]						= lst;
		if(idx!=e_shop)
		{
			AttachChild					(lst);
			lst->SetAutoDelete			(true);
		}
		CUIXmlInitGame::InitDragDropListEx	(xml_doc, _list_names[idx], 0, lst);
		BindDragDropListEvents			(lst, true);
	}

	m_static_player_money				= new CUIStatic(); AttachChild(m_static_player_money); m_static_player_money->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_player_money",					0, m_static_player_money);

	m_static_curr_items_money			= new CUIStatic(); AttachChild(m_static_curr_items_money); m_static_curr_items_money->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_curr_items_money",					0, m_static_curr_items_money);
	
// preset money indicators
	m_static_preset_money[0]			= new CUIStatic(); AttachChild(m_static_preset_money[0]); m_static_preset_money[0]->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_preset_money_last",					0, m_static_preset_money[0]);
	m_static_preset_money[1]			= new CUIStatic(); AttachChild(m_static_preset_money[1]); m_static_preset_money[1]->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_preset_money_1",					0, m_static_preset_money[1]);
	m_static_preset_money[2]				= new CUIStatic(); AttachChild(m_static_preset_money[2]); m_static_preset_money[2]->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_preset_money_2",					0, m_static_preset_money[2]);
	m_static_preset_money[3]			= new CUIStatic(); AttachChild(m_static_preset_money[3]); m_static_preset_money[3]->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_preset_money_3",					0, m_static_preset_money[3]);
	m_static_preset_money[4]			= new CUIStatic(); AttachChild(m_static_preset_money[4]); m_static_preset_money[4]->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_preset_money_def",					0, m_static_preset_money[4]);
// preset money indicators

	m_item_color_restr_rank					= CUIXmlInit::GetColor	(xml_doc, "item_color_restr_rank",	0, color_rgba(255,255,255,255));
	m_item_color_restr_money				= CUIXmlInit::GetColor	(xml_doc, "item_color_restr_money",	0, color_rgba(255,255,255,255));
	m_item_color_normal						= CUIXmlInit::GetColor	(xml_doc, "item_color_normal",		0, color_rgba(255,255,255,255));
	
	m_text_color_money_positive				= CUIXmlInit::GetColor	(xml_doc, "money_color_positive",	0, color_rgba(255,255,255,255));
	m_text_color_money_negative				= CUIXmlInit::GetColor	(xml_doc, "money_color_negative",	0, color_rgba(255,255,255,255));

	m_static_player_rank				= new CUIStatic(); AttachChild(m_static_player_rank); m_static_player_rank->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_player_rank",					0, m_static_player_rank);

	m_static_item_rank					= new CUIStatic(); AttachChild(m_static_item_rank); m_static_item_rank->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_item_rank",					0, m_static_item_rank);

	m_static_information				= new CUIStatic(); AttachChild(m_static_information); m_static_information->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_info",					0, m_static_information);

	m_static_money_change				= new CUIStatic(); AttachChild(m_static_money_change); m_static_money_change->SetAutoDelete(true);
	CUIXmlInit::InitStatic				(xml_doc, "static_money_change",					0, m_static_money_change);
	
	m_item_info							= new CUIItemInfo();
	AttachChild							(m_item_info); m_item_info->SetAutoDelete(true);
	m_item_info->Init					(0, 0, 100, 100, "buy_menu_item.xml");

	m_item_mngr							= new CItemMgr();
	m_item_mngr->Load					(sectionPrice);
	m_item_mngr->Dump					();
	SetRank								(0);
	UpdateShop							();
	SetCurrentItem						(nullptr);
}