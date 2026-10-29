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

function(rottools_read_svg_as_cpp_literals SVG_PATH OUTPUT_VARIABLE)
    file(READ "${SVG_PATH}" SVG_CONTENT)
    string(LENGTH "${SVG_CONTENT}" SVG_LENGTH)

    set(CPP_LITERALS "")
    set(SVG_OFFSET 0)
    set(SVG_CHUNK_SIZE 12000)
    while(SVG_OFFSET LESS SVG_LENGTH)
        math(EXPR SVG_REMAINING "${SVG_LENGTH} - ${SVG_OFFSET}")
        if(SVG_REMAINING LESS SVG_CHUNK_SIZE)
            set(CURRENT_CHUNK_SIZE "${SVG_REMAINING}")
        else()
            set(CURRENT_CHUNK_SIZE "${SVG_CHUNK_SIZE}")
        endif()

        string(SUBSTRING "${SVG_CONTENT}" "${SVG_OFFSET}" "${CURRENT_CHUNK_SIZE}" SVG_CHUNK)
        string(APPEND CPP_LITERALS "R\"rottools_svg(${SVG_CHUNK})rottools_svg\"\n")
        math(EXPR SVG_OFFSET "${SVG_OFFSET} + ${CURRENT_CHUNK_SIZE}")
    endwhile()

    set("${OUTPUT_VARIABLE}" "${CPP_LITERALS}" PARENT_SCOPE)
endfunction()

rottools_read_svg_as_cpp_literals("${ROTTOOLS_DEFAULT_FILE_SVG_PATH}" ROTTOOLS_DEFAULT_FILE_SVG)
rottools_read_svg_as_cpp_literals("${ROTTOOLS_MARKDOWN_FILE_SVG_PATH}" ROTTOOLS_MARKDOWN_FILE_SVG)
rottools_read_svg_as_cpp_literals("${ROTTOOLS_FOLDER_SVG_PATH}" ROTTOOLS_FOLDER_SVG)

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
