///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-85-gdf26f269)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MainFrameWx.h"

///////////////////////////////////////////////////////////////////////////

MainFrameWx::MainFrameWx( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

	MenuBar = new wxMenuBar( 0 );
	m_menu1 = new wxMenu();
	wxMenuItem* OpenFileMenuItem;
	OpenFileMenuItem = new wxMenuItem( m_menu1, wxID_OPEN, wxString( _("&Open...\tCtrl+O") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( OpenFileMenuItem );

	MenuBar->Append( m_menu1, _("File") );

	this->SetMenuBar( MenuBar );

	wxBoxSizer* MainFrameSizer;
	MainFrameSizer = new wxBoxSizer( wxHORIZONTAL );


	this->SetSizer( MainFrameSizer );
	this->Layout();
	toolBar = this->CreateToolBar( wxTB_HORIZONTAL, wxID_ANY );
	fileOpenTool = toolBar->AddTool( wxID_ANY, _("tool"), wxArtProvider::GetBitmap( wxASCII_STR(wxART_FILE_OPEN), wxASCII_STR(wxART_TOOLBAR) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	toolBar->Realize();

	statusBar = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );

	this->Centre( wxBOTH );
}

MainFrameWx::~MainFrameWx()
{
}
