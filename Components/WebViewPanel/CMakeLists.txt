# Components/WebViewPanel/CMakeLists.txt
add_library(WebViewPanel WebViewPanel.cpp WebViewPanelWx.cpp)

# Zabezpečí, že MainFrame uvidí hlavičku WebViewPanel.h
target_include_directories(WebViewPanel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Linknutie pomocných funkcií
target_link_libraries(WebViewPanel PUBLIC HelperFunctions ${wxWidgets_LIBRARIES})