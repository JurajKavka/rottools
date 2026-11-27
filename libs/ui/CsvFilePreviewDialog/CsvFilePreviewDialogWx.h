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
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class CsvFilePreviewDialogWx
///////////////////////////////////////////////////////////////////////////////
class CsvFilePreviewDialogWx : public wxDialog
{
	private:

	protected:
		wxToolBar* m_mainToolbar;
		wxCheckBox* m_firstLineAsHeaderCheckBox;
		wxGrid* m_grid1;

	public:

		CsvFilePreviewDialogWx( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 455,321 ), long style = wxDEFAULT_DIALOG_STYLE );

		~CsvFilePreviewDialogWx();

};

