# Shared chess values used by the game model and board UI.
add_library(rottools_chess_types INTERFACE)
add_library(rottools::chess_types ALIAS rottools_chess_types)

target_include_directories(rottools_chess_types INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_chess_types INTERFACE cxx_std_20)
