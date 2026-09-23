///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "FileBrowserTreePanelWx.h"

///////////////////////////////////////////////////////////////////////////

FileBrowserTreePanelWx::FileBrowserTreePanelWx( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	this->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_toolBar2 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	m_toolBar2->SetToolBitmapSize( wxSize( 16,16 ) );
	m_toolBar2->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	m_toolBar2->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

	m_homeTool = m_toolBar2->AddTool( wxID_HOME_TOOL, _("Home"), wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_HOME), wxASCII_STR(wxART_TOOLBAR) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	m_closeTool = m_toolBar2->AddTool( wxID_CLOSE_TOOL, _("Close"), wxArtProvider::GetBitmap( wxASCII_STR(wxART_CLOSE), wxASCII_STR(wxART_TOOLBAR) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	m_toolBar2->Realize();

	bSizer1->Add( m_toolBar2, 0, wxEXPAND, 0 );

	m_dataViewTreeCtrl1 = new wxDataViewTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER|wxDV_ROW_LINES );
	bSizer1->Add( m_dataViewTreeCtrl1, 1, wxEXPAND, 2 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_hiddenFilesCheckbox = new wxCheckBox( this, wxID_ANY, _("Hidden files"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_hiddenFilesCheckbox, 0, wxALL, 0 );

	wxArrayString m_fileTypeChoiceChoices;
	m_fileTypeChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_fileTypeChoiceChoices, 0 );
	m_fileTypeChoice->SetSelection( 0 );
	fgSizer1->Add( m_fileTypeChoice, 0, wxALL, 0 );


	bSizer2->Add( fgSizer1, 1, wxBOTTOM|wxEXPAND|wxTOP, 4 );


	bSizer1->Add( bSizer2, 0, wxALL|wxEXPAND, 0 );


	this->SetSizer( bSizer1 );
	this->Layout();
}

FileBrowserTreePanelWx::~FileBrowserTreePanelWx()
{
}
