# Tools in the suite. Each is guarded by its own option so a build can target a
# single tool (e.g. -DROTTOOLS_BUILD_ROTREADER=ON and everything else OFF).
if(ROTTOOLS_BUILD_ROTREADER)
    add_subdirectory(rotreader)
endif()

if(ROTTOOLS_BUILD_ROTPAD)
    add_subdirectory(rotpad)
endif()
