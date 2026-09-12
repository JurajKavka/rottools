#include "MainFrame.h"

#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statusbr.h>
#include <wx/stockitem.h>

#include <functional>

#if defined(ROTTOOLS_HAS_EMBEDDED_APP_ICON) && !defined(__WXOSX__)
#include "AppIcon.h"
#include "AppIconData.h"
#endif
#include "ui/ChessBoardPanel.h"
#include "version.h"

namespace {
constexpr auto kApplicationTitle = "ROT Chess";

[[nodiscard]] wxString ColorName(rottools::chess::Color color) {
    return color == rottools::chess::Color::White ? _("White") : _("Black");
}
}  // namespace

MainFrame::MainFrame(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, kApplicationTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE) {
#if defined(ROTTOOLS_HAS_EMBEDDED_APP_ICON) && !defined(__WXOSX__)
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif

    auto* gameMenu = new wxMenu();
    gameMenu->Append(wxID_NEW, _("&New Game\tCtrl+N"));
    gameMenu->AppendSeparator();
    gameMenu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT, wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR));

    auto* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT, wxSTOCK_WITH_MNEMONIC));

    auto* menuBar = new wxMenuBar();
    menuBar->Append(gameMenu, _("&Game"));
    menuBar->Append(helpMenu, _("&Help"));
    SetMenuBar(menuBar);

    CreateStatusBar(1, wxSTB_DEFAULT_STYLE);

    m_boardPanel = new ChessBoardPanel(this, m_game, std::bind_front(&MainFrame::HandleGameChanged, this));
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_boardPanel, 1, wxEXPAND);
    SetSizer(mainSizer);

    Bind(wxEVT_MENU, &MainFrame::HandleNewGameMenuItemClick, this, wxID_NEW);
    Bind(wxEVT_MENU, &MainFrame::HandleExitMenuItemClick, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainFrame::HandleAboutMenuItemClick, this, wxID_ABOUT);

    SetMinClientSize(FromDIP(wxSize(288, 312)));
    SetClientSize(FromDIP(wxSize(376, 400)));
    Centre();
    UpdateStatus();
}

void MainFrame::HandleNewGameMenuItemClick(wxCommandEvent&) {
    m_game.Reset();
    m_boardPanel->ResetInteraction();
    UpdateStatus();
}

void MainFrame::HandleExitMenuItemClick(wxCommandEvent&) {
    Close();
}

void MainFrame::HandleAboutMenuItemClick(wxCommandEvent&) {
    wxAboutDialogInfo info;
    info.SetName(kApplicationTitle);
    info.SetVersion(ROTCHESS_VERSION_STRING);
    info.SetDescription(_("A compact native chess board for two local players."));
    wxAboutBox(info, this);
}

void MainFrame::HandleGameChanged() {
    UpdateStatus();
}

void MainFrame::UpdateStatus() {
    using rottools::chess::GameStatus;

    wxString status;
    if (m_game.Status() == GameStatus::Checkmate) {
        status = wxString::Format(_("Checkmate — %s wins"), ColorName(*m_game.Winner()));
    } else if (m_game.Status() == GameStatus::Stalemate) {
        status = _("Stalemate");
    } else {
        status = wxString::Format(_("%s to move"), ColorName(m_game.SideToMove()));
        if (m_game.IsInCheck(m_game.SideToMove())) {
            status += _(" — check");
        }
    }
    SetStatusText(status);
}
