///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-85-gdf26f269)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "TextFilePreviewDialogWx.h"

///////////////////////////////////////////////////////////////////////////

TextFilePreviewDialogWx::TextFilePreviewDialogWx( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_toolBar1 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	m_wrapCheckBox = new wxCheckBox( m_toolBar1, wxID_ANY, _("Word Wrap"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( m_wrapCheckBox );
	m_toolBar1->Realize();

	bSizer1->Add( m_toolBar1, 0, wxEXPAND, 4 );

	m_textCtrl2 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_READONLY );
	bSizer1->Add( m_textCtrl2, 1, wxEXPAND, 4 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );
}

TextFilePreviewDialogWx::~TextFilePreviewDialogWx()
{
}
