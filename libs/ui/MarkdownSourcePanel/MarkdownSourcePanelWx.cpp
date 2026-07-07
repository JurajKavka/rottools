///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-85-gdf26f269)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MarkdownSourcePanelWx.h"

///////////////////////////////////////////////////////////////////////////

MarkdownSourcePanelWx::MarkdownSourcePanelWx( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_styledTextCtrl = new wxStyledTextCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString );
	m_styledTextCtrl->SetUseTabs( true );
	m_styledTextCtrl->SetTabWidth( 4 );
	m_styledTextCtrl->SetIndent( 4 );
	m_styledTextCtrl->SetTabIndents( true );
	m_styledTextCtrl->SetBackSpaceUnIndents( true );
	m_styledTextCtrl->SetViewEOL( false );
	m_styledTextCtrl->SetViewWhiteSpace( false );
	m_styledTextCtrl->SetMarginWidth( 2, 0 );
	m_styledTextCtrl->SetIndentationGuides( true );
	m_styledTextCtrl->SetReadOnly( false );
	m_styledTextCtrl->SetMarginType( 1, wxSTC_MARGIN_SYMBOL );
	m_styledTextCtrl->SetMarginMask( 1, wxSTC_MASK_FOLDERS );
	m_styledTextCtrl->SetMarginWidth( 1, 16);
	m_styledTextCtrl->SetMarginSensitive( 1, true );
	m_styledTextCtrl->SetProperty( wxT("fold"), wxT("1") );
	m_styledTextCtrl->SetFoldFlags( wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED );
	m_styledTextCtrl->SetMarginType( 0, wxSTC_MARGIN_NUMBER );
	m_styledTextCtrl->SetMarginWidth( 0, m_styledTextCtrl->TextWidth( wxSTC_STYLE_LINENUMBER, wxT("_99999") ) );
	m_styledTextCtrl->MarkerDefine( wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS );
	m_styledTextCtrl->MarkerSetBackground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("BLACK") ) );
	m_styledTextCtrl->MarkerSetForeground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("WHITE") ) );
	m_styledTextCtrl->MarkerDefine( wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS );
	m_styledTextCtrl->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("BLACK") ) );
	m_styledTextCtrl->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("WHITE") ) );
	m_styledTextCtrl->MarkerDefine( wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY );
	m_styledTextCtrl->MarkerDefine( wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS );
	m_styledTextCtrl->MarkerSetBackground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("BLACK") ) );
	m_styledTextCtrl->MarkerSetForeground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("WHITE") ) );
	m_styledTextCtrl->MarkerDefine( wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS );
	m_styledTextCtrl->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("BLACK") ) );
	m_styledTextCtrl->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("WHITE") ) );
	m_styledTextCtrl->MarkerDefine( wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY );
	m_styledTextCtrl->MarkerDefine( wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY );
	m_styledTextCtrl->SetSelBackground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_styledTextCtrl->SetSelForeground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) );
	bSizer1->Add( m_styledTextCtrl, 1, wxEXPAND | wxALL, 0 );


	this->SetSizer( bSizer1 );
	this->Layout();
}

MarkdownSourcePanelWx::~MarkdownSourcePanelWx()
{
}
