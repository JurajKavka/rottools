///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-85-gdf26f269)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameWx
///////////////////////////////////////////////////////////////////////////////
class MainFrameWx : public wxFrame
{
	private:

	protected:
		enum
		{
			wxID_NEW_WINDOW_MENU_ITEM = 6000,
			wxID_SOLO_WEB_VIEW_PANEL_MENU_ITEM,
			wxID_TOGGLE_FILE_BROWSER_MENU_ITEM,
			wxID_TOGGLE_HTML_SOURCE_PANEL_MENU_ITEM,
			wxID_TOGGLE_MARKDOWN_SOURCE_PANEL_MENU_ITEM,
		};

		wxMenuBar* MenuBar;
		wxMenu* m_menu1;
		wxMenu* m_menu2;
		wxMenu* m_themeSubmenu;
		wxToolBar* toolBar;
		wxToolBarToolBase* fileOpenTool;
		wxStatusBar* statusBar;

	public:

		MainFrameWx( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("ℜ⛤𝔗 pad"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1051,642 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~MainFrameWx();

};

