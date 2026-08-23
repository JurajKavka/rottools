# Select the platform-native implementations. The public API and worker are
# common; native headers remain private to this target.
find_package(Threads REQUIRED)

if(APPLE)
    enable_language(OBJCXX)
    set(ROTTOOLS_NATIVE_FILE_COPY_SOURCE Native/NativeFileCopyMac.cpp)
    set(ROTTOOLS_NATIVE_FILE_CREATE_SOURCE Native/NativeFileCreatePosix.cpp)
    set(ROTTOOLS_NATIVE_FILE_MOVE_SOURCE Native/NativeFileMoveMac.cpp)
    set(ROTTOOLS_NATIVE_FILE_TRASH_SOURCE Native/NativeFileTrashMac.mm)
elseif(WIN32)
    set(ROTTOOLS_NATIVE_FILE_COPY_SOURCE Native/NativeFileCopyWindows.cpp)
    set(ROTTOOLS_NATIVE_FILE_CREATE_SOURCE Native/NativeFileCreateWindows.cpp)
    set(ROTTOOLS_NATIVE_FILE_MOVE_SOURCE Native/NativeFileMoveWindows.cpp)
    set(ROTTOOLS_NATIVE_FILE_TRASH_SOURCE Native/NativeFileTrashWindows.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ROTTOOLS_GIO REQUIRED IMPORTED_TARGET gio-2.0)
    set(ROTTOOLS_NATIVE_FILE_COPY_SOURCE Native/NativeFileCopyLinux.cpp)
    set(ROTTOOLS_NATIVE_FILE_CREATE_SOURCE Native/NativeFileCreatePosix.cpp)
    set(ROTTOOLS_NATIVE_FILE_MOVE_SOURCE Native/NativeFileMoveLinux.cpp)
    set(ROTTOOLS_NATIVE_FILE_TRASH_SOURCE Native/NativeFileTrashLinux.cpp)
else()
    message(FATAL_ERROR "rottools: no native file-operation backend for ${CMAKE_SYSTEM_NAME}")
endif()

# rottools::fileops
#   layer 1: FileOperations + private Native implementations
#   layer 2: FileOperationTask
#   layer 3: FileClipboard (request creation only)
add_library(rottools_fileops STATIC
    FileOperations.cpp
    FileOperationTask.cpp
    FileClipboard.cpp
    ${ROTTOOLS_NATIVE_FILE_COPY_SOURCE}
    ${ROTTOOLS_NATIVE_FILE_CREATE_SOURCE}
    ${ROTTOOLS_NATIVE_FILE_MOVE_SOURCE}
    ${ROTTOOLS_NATIVE_FILE_TRASH_SOURCE}
)
add_library(rottools::fileops ALIAS rottools_fileops)

target_include_directories(rottools_fileops
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Native
)
target_compile_features(rottools_fileops PUBLIC cxx_std_20)
target_link_libraries(rottools_fileops PRIVATE Threads::Threads)

if(APPLE)
    find_library(ROTTOOLS_APPKIT_FRAMEWORK AppKit REQUIRED)
    target_link_libraries(rottools_fileops PRIVATE "${ROTTOOLS_APPKIT_FRAMEWORK}")
    set_target_properties(rottools_fileops PROPERTIES
        OBJCXX_STANDARD 20
        OBJCXX_STANDARD_REQUIRED YES
        OBJCXX_EXTENSIONS NO
    )
elseif(WIN32)
    target_link_libraries(rottools_fileops PRIVATE ole32 shell32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(rottools_fileops PRIVATE PkgConfig::ROTTOOLS_GIO)
endif()
