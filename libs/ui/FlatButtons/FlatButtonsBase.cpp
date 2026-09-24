#include "FlatButtonsBase.h"

#include <wx/dcbuffer.h>
#include <wx/settings.h>

#include <utility>

FlatButtonsBase::FlatButtonsBase(wxWindow* parent, const wxString& label, std::function<void()> onClickHandler,
                                 wxWindowID id)
    : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxWANTS_CHARS),
      m_onClickHandler(std::move(onClickHandler)) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetLabel(label);
    Bind(wxEVT_BUTTON, &FlatButtonsBase::HandleClick, this);
    Bind(wxEVT_PAINT, &FlatButtonsBase::HandlePaint, this);
    Bind(wxEVT_ENTER_WINDOW, &FlatButtonsBase::HandleEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &FlatButtonsBase::HandleLeave, this);
    Bind(wxEVT_LEFT_DOWN, &FlatButtonsBase::HandleLeftDown, this);
    Bind(wxEVT_LEFT_UP, &FlatButtonsBase::HandleLeftUp, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &FlatButtonsBase::HandleCaptureLost, this);
    Bind(wxEVT_KEY_DOWN, &FlatButtonsBase::HandleKeyDown, this);
    Bind(wxEVT_SET_FOCUS, &FlatButtonsBase::HandleFocusChanged, this);
    Bind(wxEVT_KILL_FOCUS, &FlatButtonsBase::HandleFocusChanged, this);
    Bind(wxEVT_SYS_COLOUR_CHANGED, &FlatButtonsBase::HandleSystemColourChanged, this);
}

wxSize FlatButtonsBase::DoGetBestSize() const {
    return FromDIP(wxSize(24, 24));
}

wxColour FlatButtonsBase::BlendColours(const wxColour& background, const wxColour& foreground, int foregroundAlpha) {
    const int backgroundAlpha = 255 - foregroundAlpha;
    return wxColour((background.Red() * backgroundAlpha + foreground.Red() * foregroundAlpha) / 255,
                    (background.Green() * backgroundAlpha + foreground.Green() * foregroundAlpha) / 255,
                    (background.Blue() * backgroundAlpha + foreground.Blue() * foregroundAlpha) / 255);
}

void FlatButtonsBase::Activate() {
    wxCommandEvent click(wxEVT_BUTTON, GetId());
    click.SetEventObject(this);
    ProcessWindowEvent(click);
}

void FlatButtonsBase::HandleClick(wxCommandEvent& event) {
    auto onClickHandler = m_onClickHandler;
    event.Skip();
    if (onClickHandler) {
        onClickHandler();
    }
}

void FlatButtonsBase::HandlePaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    const wxColour background = GetParent()->GetBackgroundColour();
    const wxColour textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    const wxColour foreground = m_hovered ? textColour : BlendColours(background, textColour, 180);
    dc.SetBackground(wxBrush(background));
    dc.Clear();

    wxRect buttonRect = GetClientRect();
    buttonRect.Deflate(FromDIP(2));
    dc.SetPen(wxPen(foreground, FromDIP(1)));
    const wxSize size = GetClientSize();
    DrawIcon(dc, wxPoint(size.x / 2, size.y / 2));

    if (HasFocus()) {
        dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT), FromDIP(1)));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(buttonRect, FromDIP(4));
    }
}

void FlatButtonsBase::HandleEnter(wxMouseEvent& event) {
    m_hovered = true;
    Refresh();
}

void FlatButtonsBase::HandleLeave(wxMouseEvent& event) {
    m_hovered = false;
    Refresh();
}

void FlatButtonsBase::HandleLeftDown(wxMouseEvent& event) {
    SetFocus();
    CaptureMouse();
    m_pressed = true;
    m_hovered = true;
    Refresh();
}

void FlatButtonsBase::HandleLeftUp(wxMouseEvent& event) {
    const bool activate = m_pressed && GetClientRect().Contains(event.GetPosition());
    m_pressed = false;
    if (HasCapture()) {
        ReleaseMouse();
    }
    Refresh();
    if (activate) {
        Activate();
    }
}

void FlatButtonsBase::HandleCaptureLost(wxMouseCaptureLostEvent& event) {
    m_pressed = false;
    Refresh();
}

void FlatButtonsBase::HandleKeyDown(wxKeyEvent& event) {
    const int key = event.GetKeyCode();
    if (key == WXK_SPACE || key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
        Activate();
    } else {
        event.Skip();
    }
}

void FlatButtonsBase::HandleFocusChanged(wxFocusEvent& event) {
    Refresh();
    event.Skip();
}

void FlatButtonsBase::HandleSystemColourChanged(wxSysColourChangedEvent& event) {
    Refresh();
    event.Skip();
}
