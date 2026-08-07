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
	m_fileMenu = new wxMenu();
	wxMenuItem* OpenFileMenuItem;
	OpenFileMenuItem = new wxMenuItem( m_fileMenu, wxID_OPEN, wxString( _("&Open...") ) , wxEmptyString, wxITEM_NORMAL );
	m_fileMenu->Append( OpenFileMenuItem );

	wxMenuItem* m_newWindowMenuItem;
	m_newWindowMenuItem = new wxMenuItem( m_fileMenu, wxID_NEW_WINDOW_MENU_ITEM, wxString( _("New Window\tCtrl+N") ) , wxEmptyString, wxITEM_NORMAL );
	m_fileMenu->Append( m_newWindowMenuItem );

	m_fileMenu->AppendSeparator();

	wxMenuItem* m_saveMenuItem;
	m_saveMenuItem = new wxMenuItem( m_fileMenu, wxID_SAVE, wxString( _("&Save") ) + wxT('\t') + wxT("CTRL+S"), wxEmptyString, wxITEM_NORMAL );
	m_fileMenu->Append( m_saveMenuItem );

	wxMenuItem* m_saveAsMenuItem;
	m_saveAsMenuItem = new wxMenuItem( m_fileMenu, wxID_SAVEAS, wxString( _("Save As...") ) + wxT('\t') + wxT("CTRL+SHIFT+S"), wxEmptyString, wxITEM_NORMAL );
	m_fileMenu->Append( m_saveAsMenuItem );

	MenuBar->Append( m_fileMenu, _("File") );

	m_editMenu = new wxMenu();
	wxMenuItem* m_undoMenuItem;
	m_undoMenuItem = new wxMenuItem( m_editMenu, wxID_UNDO, wxString( _("Undo") ) + wxT('\t') + wxT("CTRL+Z"), wxEmptyString, wxITEM_NORMAL );
	m_editMenu->Append( m_undoMenuItem );

	wxMenuItem* m_redoMenuItem;
	m_redoMenuItem = new wxMenuItem( m_editMenu, wxID_REDO, wxString( _("Redo") ) , wxEmptyString, wxITEM_NORMAL );
	m_editMenu->Append( m_redoMenuItem );

	m_editMenu->AppendSeparator();

	wxMenuItem* m_copyMenuItem;
	m_copyMenuItem = new wxMenuItem( m_editMenu, wxID_COPY, wxString( _("Copy") ) + wxT('\t') + wxT("CTRL+C"), wxEmptyString, wxITEM_NORMAL );
	m_editMenu->Append( m_copyMenuItem );

	wxMenuItem* m_cutMenuItem;
	m_cutMenuItem = new wxMenuItem( m_editMenu, wxID_CUT, wxString( _("Cut") ) + wxT('\t') + wxT("CTRL+X"), wxEmptyString, wxITEM_NORMAL );
	m_editMenu->Append( m_cutMenuItem );

	wxMenuItem* m_pasteMenuItem;
	m_pasteMenuItem = new wxMenuItem( m_editMenu, wxID_PASTE, wxString( _("Paste") ) , wxEmptyString, wxITEM_NORMAL );
	m_editMenu->Append( m_pasteMenuItem );

	MenuBar->Append( m_editMenu, _("Edit") );

	m_viewMenu = new wxMenu();
	wxMenuItem* m_toggleFileBrowserMenuItem;
	m_toggleFileBrowserMenuItem = new wxMenuItem( m_viewMenu, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM, wxString( _("Toggle file browser") ) , wxEmptyString, wxITEM_NORMAL );
	m_viewMenu->Append( m_toggleFileBrowserMenuItem );

	wxMenuItem* m_wordWrap;
	m_wordWrap = new wxMenuItem( m_viewMenu, wxID_WORDWRAP, wxString( _("Word Wrap") ) , wxEmptyString, wxITEM_CHECK );
	m_viewMenu->Append( m_wordWrap );

	MenuBar->Append( m_viewMenu, _("View") );

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
