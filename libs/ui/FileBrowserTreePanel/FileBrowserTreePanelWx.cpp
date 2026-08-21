///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-85-gdf26f269)
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

	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel1->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	m_bpButton1 = new wxBitmapButton( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 16,16 ), wxBU_AUTODRAW|0 );

	m_bpButton1->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_BACK), wxASCII_STR(wxART_BUTTON) ) );
	m_bpButton1->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	m_bpButton1->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	m_bpButton1->SetToolTip( _("Directory of previous opened file.") );

	bSizer3->Add( m_bpButton1, 0, wxRIGHT, 2 );

	m_bpButton11 = new wxBitmapButton( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 16,16 ), wxBU_AUTODRAW|0 );

	m_bpButton11->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_FORWARD), wxASCII_STR(wxART_BUTTON) ) );
	m_bpButton11->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_SCROLLBAR ) );
	m_bpButton11->SetToolTip( _("Directory of next opened file.") );

	bSizer3->Add( m_bpButton11, 0, wxRIGHT, 2 );

	m_bpButton111 = new wxBitmapButton( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 16,16 ), wxBU_AUTODRAW|0 );

	m_bpButton111->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_HOME), wxASCII_STR(wxART_BUTTON) ) );
	m_bpButton111->SetBitmapDisabled( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_HOME), wxASCII_STR(wxART_BUTTON) ) );
	m_bpButton111->SetToolTip( _("Users home directory.") );

	bSizer3->Add( m_bpButton111, 0, wxRIGHT, 2 );

	m_bpButton1111 = new wxBitmapButton( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 16,16 ), wxBU_AUTODRAW|0 );

	m_bpButton1111->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CLOSE), wxASCII_STR(wxART_BUTTON) ) );
	m_bpButton1111->SetBitmapDisabled( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CLOSE), wxASCII_STR(wxART_BUTTON) ) );
	m_bpButton1111->SetToolTip( _("Users home directory.") );

	bSizer3->Add( m_bpButton1111, 0, wxRIGHT, 2 );


	m_panel1->SetSizer( bSizer3 );
	m_panel1->Layout();
	bSizer3->Fit( m_panel1 );
	bSizer1->Add( m_panel1, 0, wxALIGN_RIGHT|wxALL, 4 );

	m_dataViewTreeCtrl1 = new wxDataViewTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER|wxDV_ROW_LINES );
	bSizer1->Add( m_dataViewTreeCtrl1, 1, wxEXPAND, 0 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_hiddenFilesCheckbox = new wxCheckBox( this, wxID_ANY, _("Hidden files"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_hiddenFilesCheckbox, 0, wxALL, 0 );


	bSizer1->Add( bSizer2, 0, wxALL|wxEXPAND, 8 );


	this->SetSizer( bSizer1 );
	this->Layout();
}

FileBrowserTreePanelWx::~FileBrowserTreePanelWx()
{
}
