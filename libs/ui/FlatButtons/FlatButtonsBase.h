#pragma once

#include <wx/colour.h>
#include <wx/control.h>

#include <functional>

class wxDC;

class FlatButtonsBase : public wxControl {
   protected:
    FlatButtonsBase(wxWindow* parent, const wxString& label, std::function<void()> onClickHandler,
                    wxWindowID id = wxID_ANY);

    wxSize DoGetBestSize() const override;
    virtual void DrawIcon(wxDC& dc, const wxPoint& center) const = 0;

   private:
    bool m_hovered = false;
    bool m_pressed = false;
    std::function<void()> m_onClickHandler;

    static wxColour BlendColours(const wxColour& background, const wxColour& foreground, int foregroundAlpha);

    void Activate();
    void HandleClick(wxCommandEvent& event);
    void HandlePaint(wxPaintEvent& event);
    void HandleEnter(wxMouseEvent& event);
    void HandleLeave(wxMouseEvent& event);
    void HandleLeftDown(wxMouseEvent& event);
    void HandleLeftUp(wxMouseEvent& event);
    void HandleCaptureLost(wxMouseCaptureLostEvent& event);
    void HandleKeyDown(wxKeyEvent& event);
    void HandleFocusChanged(wxFocusEvent& event);
    void HandleSystemColourChanged(wxSysColourChangedEvent& event);
};
