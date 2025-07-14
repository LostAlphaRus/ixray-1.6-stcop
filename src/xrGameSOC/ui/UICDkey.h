//
#pragma once
#include "../../xrUI/Widgets/UIEditBox.h"

class CUICDkey : public CUIEditBox
{
private:
	typedef			CUIEditBox		inherited;

public:
					CUICDkey		();
	virtual	void	SetText			(LPCSTR str) {}
	virtual	LPCSTR	GetText			();

	// CUIOptionsItem
	virtual void	SetCurrentOptValue	();	// opt->current
	virtual void	SaveBackUpOptValue	();	// current->backup
	virtual void	SaveOptValue		();	// current->opt
	virtual void	UndoOptValue		();	// backup->current
	virtual bool	IsChangedOptValue	() const;	// backup!=current
	
			void	CreateCDKeyEntry();			
			void	assign_callbacks();

	virtual void	Show			(bool status);
	virtual void	Draw			();
	virtual void	OnFocusLost		();

private:
	void 	paste_from_clipboard();

private:
	string512		m_opt_backup_value;
	bool			m_view_access;
}; // class CUICDkey


extern	void	GetCDKey(char* CDKeyStr);