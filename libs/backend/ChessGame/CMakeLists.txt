# rottools::chessgame — UI-independent chess state and legal move validation.
add_library(rottools_chessgame STATIC ChessGame.cpp)
add_library(rottools::chessgame ALIAS rottools_chessgame)

target_include_directories(rottools_chessgame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_chessgame PUBLIC cxx_std_20)
