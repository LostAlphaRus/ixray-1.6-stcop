////////////////////////////////////////////////////////////////////////////
//	Module 		: UIHelper.cpp
//	Created 	: 17.01.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Helper class implementation
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIHelperGame.h"
#include "../xrUI/UIXmlInit.h"

#include "../xrUI/Widgets/uilistwnd.h"
#include "ui/UILabel.h"
#include "../xrEngine/string_table.h"
#include "ui/UIDragDropListEx.h"

CUIDragDropListEx* UIHelperGame::CreateDragDropListEx(CUIXml& xml, LPCSTR ui_path, CUIWindow* parent)
{
	CUIDragDropListEx* ui = new CUIDragDropListEx();
	parent->AttachChild(ui);
	ui->SetAutoDelete(true);
	CUIXmlInitGame::InitDragDropListEx(xml, ui_path, 0, ui);
	return ui;
}

bool CUIXmlInitGame::InitDragDropListEx(CUIXml& xml_doc, LPCSTR path, int index, CUIDragDropListEx* pWnd)
{

	R_ASSERT3(xml_doc.NavigateToNode(path,index), "XML node not found", path);

	float x			= xml_doc.ReadAttribFlt(path, index, "x");
	float y			= xml_doc.ReadAttribFlt(path, index, "y");
	float width		= xml_doc.ReadAttribFlt(path, index, "width");
	float height	= xml_doc.ReadAttribFlt(path, index, "height");

	InitAlignment	(xml_doc, path, index, x, y, pWnd);



	pWnd->Init		(x,y, width,height);

	Ivector2 w_cell_sz, w_cells;

	w_cell_sz.x				= xml_doc.ReadAttribInt(path, index, "cell_width");
	w_cell_sz.y				= xml_doc.ReadAttribInt(path, index, "cell_height");
	w_cells.y				= xml_doc.ReadAttribInt(path, index, "rows_num");
	w_cells.x				= xml_doc.ReadAttribInt(path, index, "cols_num");
	pWnd->SetCellSize		(w_cell_sz);	
	pWnd->SetStartCellsCapacity	(w_cells);	

	int tmp					= xml_doc.ReadAttribInt(path, index, "unlimited", 0);
	pWnd->SetAutoGrow		(tmp!=0);
	tmp						= xml_doc.ReadAttribInt(path, index, "group_similar", 0);
	pWnd->SetGrouping		(tmp!=0);
	tmp						= xml_doc.ReadAttribInt(path, index, "custom_placement", 1);
	pWnd->SetCustomPlacement(tmp!=0);

	return true;
}

bool CUIXmlInitGame::InitLabel(CUIXml& xml_doc, LPCSTR path, int index, CUILabel* pWnd)
{
	InitFrameLine(xml_doc, path, index, pWnd);

	string256 buf;
	xr_strconcat(buf, path, ":text");
	InitText(xml_doc, buf, index, &pWnd->m_text);

	return true;
}
