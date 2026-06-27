cmake_minimum_required(VERSION 4.0)

include(FetchContent)
FetchContent_Declare(
    md4c
    GIT_REPOSITORY https://github.com/mity/md4c.git
    GIT_TAG        release-0.5.3
)
FetchContent_MakeAvailable(md4c)

add_library(Backend STATIC 
     MarkdownToHtmlAsync/MarkdownToHtmlAsync.cpp
)

target_include_directories(Backend PUBLIC MarkdownToHtmlAsync)
target_link_libraries(Backend PUBLIC md4c-html)



