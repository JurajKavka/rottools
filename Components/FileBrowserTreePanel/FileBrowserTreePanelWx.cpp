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
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_bpButton1 = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE );

	m_bpButton1->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_BACK), wxASCII_STR(wxART_OTHER) ) );
	bSizer2->Add( m_bpButton1, 0, 0, 0 );

	m_bpButton2 = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE );

	m_bpButton2->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_FORWARD), wxASCII_STR(wxART_OTHER) ) );
	m_bpButton2->SetBitmapDisabled( wxNullBitmap );
	bSizer2->Add( m_bpButton2, 0, 0, 0 );


	bSizer1->Add( bSizer2, 0, wxEXPAND, 0 );

	m_dataViewTreeCtrl1 = new wxDataViewTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER|wxDV_ROW_LINES );
	bSizer1->Add( m_dataViewTreeCtrl1, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer1 );
	this->Layout();
}

FileBrowserTreePanelWx::~FileBrowserTreePanelWx()
{
}
