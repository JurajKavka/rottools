# rottools::ui_chessboard — chess board painting and mouse input callbacks.
add_library(rottools_ui_chessboard STATIC ChessBoardPanel.cpp)
add_library(rottools::ui_chessboard ALIAS rottools_ui_chessboard)

target_include_directories(rottools_ui_chessboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_chessboard PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_chessboard PUBLIC rottools::chess_types ${ROTTOOLS_WX_CORE_LIBRARIES})
