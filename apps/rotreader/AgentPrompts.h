#pragma once

#include <wx/filename.h>
#include <wx/string.h>

namespace agent_prompts {

[[nodiscard]] wxString MakeEnglishReview(const wxFileName& document);

}  // namespace agent_prompts
