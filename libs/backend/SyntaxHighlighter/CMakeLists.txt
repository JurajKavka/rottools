# rottools::syntaxhighlighter — reference format detection and Scintilla syntax data.
add_library(rottools_syntaxhighlighter STATIC SyntaxHighlighter.cpp)
add_library(rottools::syntaxhighlighter ALIAS rottools_syntaxhighlighter)

target_include_directories(rottools_syntaxhighlighter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_syntaxhighlighter PUBLIC cxx_std_20)
target_link_libraries(rottools_syntaxhighlighter PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_syntaxhighlighter_app main.cpp)
    target_link_libraries(rottools_syntaxhighlighter_app PRIVATE rottools::syntaxhighlighter)
endif()
