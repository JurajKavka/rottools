# Shared libraries, reusable across every tool in the suite.
# Each declares a rottools::<name> alias that apps link against.
add_subdirectory(HelperFunctions)
add_subdirectory(backend)
add_subdirectory(ui)
