# rottools::ui_icons — reusable SVG icons for wxWidgets controls.
set(ROTTOOLS_DEFAULT_FILE_SVG_PATH
    "${PROJECT_SOURCE_DIR}/docs/graphics/icons/generated/default-file.svg"
)
set(ROTTOOLS_MARKDOWN_FILE_SVG_PATH
    "${PROJECT_SOURCE_DIR}/docs/graphics/icons/generated/md-file.svg"
)
set(ROTTOOLS_FOLDER_SVG_PATH
    "${PROJECT_SOURCE_DIR}/docs/graphics/icons/generated/folder.svg"
)

foreach(ROTTOOLS_ICON_SVG_PATH IN ITEMS
    "${ROTTOOLS_DEFAULT_FILE_SVG_PATH}"
    "${ROTTOOLS_MARKDOWN_FILE_SVG_PATH}"
    "${ROTTOOLS_FOLDER_SVG_PATH}"
)
    if(NOT EXISTS "${ROTTOOLS_ICON_SVG_PATH}")
        message(FATAL_ERROR
            "Missing generated UI icon: ${ROTTOOLS_ICON_SVG_PATH}. "
            "Run 'make ui-icons' from the repository root."
        )
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ROTTOOLS_ICON_SVG_PATH}")
endforeach()

file(READ "${ROTTOOLS_DEFAULT_FILE_SVG_PATH}" ROTTOOLS_DEFAULT_FILE_SVG)
file(READ "${ROTTOOLS_MARKDOWN_FILE_SVG_PATH}" ROTTOOLS_MARKDOWN_FILE_SVG)
file(READ "${ROTTOOLS_FOLDER_SVG_PATH}" ROTTOOLS_FOLDER_SVG)

set(ROTTOOLS_ICONS_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${ROTTOOLS_ICONS_GENERATED_DIR}")
configure_file(
    IconsData.h.in
    "${ROTTOOLS_ICONS_GENERATED_DIR}/IconsData.h"
    @ONLY
)

add_library(rottools_ui_icons STATIC Icons.cpp)
add_library(rottools::ui_icons ALIAS rottools_ui_icons)

target_include_directories(rottools_ui_icons
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE "${ROTTOOLS_ICONS_GENERATED_DIR}"
)
target_compile_features(rottools_ui_icons PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_icons PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_icons_app main.cpp)
    target_link_libraries(rottools_ui_icons_app PRIVATE rottools::ui_icons ${wxWidgets_LIBRARIES})
endif()
