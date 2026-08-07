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
	OpenFileMenuItem = new wxMenuItem( m_menu1, wxID_OPEN, wxString( _("&Open...") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( OpenFileMenuItem );

	wxMenuItem* m_newWindowMenuItem;
	m_newWindowMenuItem = new wxMenuItem( m_menu1, wxID_NEW_WINDOW_MENU_ITEM, wxString( _("New Window\tCtrl+N") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( m_newWindowMenuItem );

	m_menu1->AppendSeparator();

	wxMenuItem* m_saveMenuItem;
	m_saveMenuItem = new wxMenuItem( m_menu1, wxID_SAVE, wxString( _("&Save") ) + wxT('\t') + wxT("CTRL+S"), wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( m_saveMenuItem );

	wxMenuItem* m_saveAsMenuItem;
	m_saveAsMenuItem = new wxMenuItem( m_menu1, wxID_SAVEAS, wxString( _("Save As...") ) + wxT('\t') + wxT("CTRL+SHIFT+S"), wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( m_saveAsMenuItem );

	MenuBar->Append( m_menu1, _("File") );

	m_menu2 = new wxMenu();
	wxMenuItem* m_toggleFileBrowserMenuItem;
	m_toggleFileBrowserMenuItem = new wxMenuItem( m_menu2, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM, wxString( _("Toggle file browser") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu2->Append( m_toggleFileBrowserMenuItem );

	MenuBar->Append( m_menu2, _("View") );

	this->SetMenuBar( MenuBar );

	wxBoxSizer* MainFrameSizer;
	MainFrameSizer = new wxBoxSizer( wxHORIZONTAL );


	this->SetSizer( MainFrameSizer );
	this->Layout();
	toolBar = this->CreateToolBar( wxTB_HORIZONTAL, wxID_ANY );
	m_fileOpenTool = toolBar->AddTool( wxID_ANY, _("Open file"), wxArtProvider::GetBitmap( wxASCII_STR(wxART_FILE_OPEN), wxASCII_STR(wxART_TOOLBAR) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	m_saveTool = toolBar->AddTool( wxID_ANY, _("tool"), wxArtProvider::GetBitmap( wxASCII_STR(wxART_FILE_SAVE), wxASCII_STR(wxART_TOOLBAR) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	m_saveAsTool = toolBar->AddTool( wxID_ANY, _("tool"), wxArtProvider::GetBitmap( wxASCII_STR(wxART_FILE_SAVE_AS), wxASCII_STR(wxART_TOOLBAR) ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL );

	toolBar->Realize();

	statusBar = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );

	this->Centre( wxBOTH );
}

MainFrameWx::~MainFrameWx()
{
}
