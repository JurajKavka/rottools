# rottools_apply_version(<target> <PREFIX> <version_file>)
#
# Reads a single X.Y.Z line from <version_file> (a tool's ./VERSION — the one
# source of truth), generates a "version.h" carrying <PREFIX>_VERSION_* macros
# into the target's private include dir, and sets ${<PREFIX>_VERSION} in the
# caller's scope so packaging (CPack) can reuse the same value.
function(rottools_apply_version TARGET PREFIX VERSION_FILE)
    if(NOT EXISTS "${VERSION_FILE}")
        message(FATAL_ERROR "rottools: version file not found: ${VERSION_FILE}")
    endif()
    file(STRINGS "${VERSION_FILE}" _lines)
    list(GET _lines 0 _ver)
    string(STRIP "${_ver}" _ver)
    if(NOT _ver MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
        message(FATAL_ERROR "rottools: '${_ver}' in ${VERSION_FILE} is not X.Y.Z")
    endif()
    set(_ver_major "${CMAKE_MATCH_1}")
    set(_ver_minor "${CMAKE_MATCH_2}")
    set(_ver_patch "${CMAKE_MATCH_3}")
    set(_ver_string "${_ver}")

    set(_out_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/version.h.in"
                   "${_out_dir}/version.h" @ONLY)
    target_include_directories(${TARGET} PRIVATE "${_out_dir}")

    set(${PREFIX}_VERSION "${_ver}" PARENT_SCOPE)
endfunction()
