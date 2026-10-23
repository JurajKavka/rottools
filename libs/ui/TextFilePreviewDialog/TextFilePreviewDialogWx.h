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
#include <wx/checkbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/toolbar.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class TextFilePreviewDialogWx
///////////////////////////////////////////////////////////////////////////////
class TextFilePreviewDialogWx : public wxDialog
{
	private:

	protected:
		wxToolBar* m_toolBar1;
		wxCheckBox* m_wrapCheckBox;
		wxTextCtrl* m_textCtrl2;

	public:

		TextFilePreviewDialogWx( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 983,610 ), long style = wxDEFAULT_DIALOG_STYLE );

		~TextFilePreviewDialogWx();

};

