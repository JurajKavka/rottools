#pragma once

#include <wx/panel.h>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "FlatButtonsBase.h"

class HeaderPanel final : public wxPanel {
   public:
    struct ToolButton {
        using CreateButton = FlatButtonsBase* (*)(wxWindow*, const wxString&, std::function<void()>);

        CreateButton create;
        wxString label;
        std::function<void()> onClickHandler;

        /** Button must derive from FlatButtonsBase and accept (parent, label, onClickHandler). */
        template <typename Button>
        static ToolButton Make(wxString label, std::function<void()> onClickHandler) {
            static_assert(std::is_base_of_v<FlatButtonsBase, Button>);
            return {
                [](wxWindow* parent, const wxString& buttonLabel, std::function<void()> handler) -> FlatButtonsBase* {
                    return new Button(parent, buttonLabel, std::move(handler));
                },
                std::move(label),
                std::move(onClickHandler),
            };
        }
    };

    struct CloseButtonConfig {
        wxString label;
        std::function<void()> onClickHandler;
    };

    HeaderPanel(wxWindow* parent, std::vector<ToolButton> toolButtons, CloseButtonConfig closeButtonConfig);
};
