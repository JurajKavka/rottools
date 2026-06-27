cmake_minimum_required(VERSION 4.0)

add_subdirectory(MarkdownToHtmlAsync)

add_library(Backend INTERFACE)

target_include_directories(Backend INTERFACE MarkdownToHtmlAsync)
target_link_libraries(Backend INTERFACE MarkdownToHtmlAsync)
