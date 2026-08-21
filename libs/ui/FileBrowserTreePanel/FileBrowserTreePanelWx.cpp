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

	m_homeButton = new wxBitmapButton( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 16,16 ), wxBU_AUTODRAW|0 );

	m_homeButton->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_HOME), wxASCII_STR(wxART_BUTTON) ) );
	m_homeButton->SetBitmapDisabled( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_HOME), wxASCII_STR(wxART_BUTTON) ) );
	m_homeButton->SetToolTip( _("Users home directory.") );

	bSizer3->Add( m_homeButton, 0, wxLEFT, 2 );

	m_closeButton = new wxBitmapButton( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 16,16 ), wxBU_AUTODRAW|0 );

	m_closeButton->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CLOSE), wxASCII_STR(wxART_BUTTON) ) );
	m_closeButton->SetBitmapDisabled( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CLOSE), wxASCII_STR(wxART_BUTTON) ) );
	m_closeButton->SetToolTip( _("Users home directory.") );

	bSizer3->Add( m_closeButton, 0, wxLEFT, 2 );


	m_panel1->SetSizer( bSizer3 );
	m_panel1->Layout();
	bSizer3->Fit( m_panel1 );
	bSizer1->Add( m_panel1, 0, wxALIGN_RIGHT|wxBOTTOM|wxTOP, 2 );

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
