# Shared libraries, reusable across every tool in the suite.
# Each declares a rottools::<name> alias that apps link against.
add_subdirectory(HelperFunctions)
if(ROTTOOLS_BUILD_ROTCHESS OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(ChessTypes)
endif()
add_subdirectory(backend)
add_subdirectory(ui)
