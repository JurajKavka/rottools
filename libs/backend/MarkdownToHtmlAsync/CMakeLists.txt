# rottools::md2html — async md4c-based Markdown -> HTML worker.

# md4c: prefer a package (vcpkg port / system), fall back to fetching the source
# so a plain system build (no vcpkg toolchain) still works.
find_package(md4c CONFIG QUIET)
if(md4c_FOUND)
    set(ROTTOOLS_MD4C_HTML md4c::md4c-html)
else()
    include(FetchContent)
    FetchContent_Declare(
        md4c
        GIT_REPOSITORY https://github.com/mity/md4c.git
        GIT_TAG        release-0.5.3
    )
    FetchContent_MakeAvailable(md4c)
    set(ROTTOOLS_MD4C_HTML md4c-html)
endif()

add_library(rottools_md2html STATIC MarkdownToHtmlAsync.cpp)
add_library(rottools::md2html ALIAS rottools_md2html)

# PUBLIC so consumers (e.g. rotreader's MainFrame.h) can #include "MarkdownToHtmlAsync.h".
target_include_directories(rottools_md2html PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_md2html PUBLIC cxx_std_20)
target_link_libraries(rottools_md2html PRIVATE ${ROTTOOLS_MD4C_HTML} rottools::helpers ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_md2html_app main.cpp)
    target_link_libraries(rottools_md2html_app PRIVATE rottools::md2html rottools::helpers ${wxWidgets_LIBRARIES})
endif()
