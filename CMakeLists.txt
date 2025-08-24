cmake_minimum_required(VERSION 4.0)
project(particle-sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ignore duplicated libraries warning; often by vcpkg
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_warn_duplicate_libraries")

# CHANGE: IMPORT PACKAGES
find_package(fmt CONFIG REQUIRED)
# END CHANGE

# get all files in src directory
file(GLOB_RECURSE FILES_FOLDERS
    LIST_DIRECTORIES true
    ${CMAKE_SOURCE_DIR}/src/*
)

# loop through all files and seperate files from directories
set(FILES "") # list
set(FOLDERS "") # list
foreach(item IN LISTS FILES_FOLDERS)
    # only include cpp and hpp files in files list
    string(REGEX MATCH "\\.cpp$" isCPPFile "${item}")
    string(REGEX MATCH "\\.hpp$" isHPPFile "${item}")
    if(IS_DIRECTORY "${item}")
        list(APPEND FOLDERS "${item}")
    elseif(isCPPFile)
        list(APPEND FILES "${item}")
    elseif(isHPPFile)
        list(APPEND FILES "${item}")
    endif()
endforeach()

# log all detected files in console
message(STATUS "Files Detected:")
foreach(f IN LISTS FILES)
    message(STATUS "${f}")
endforeach()

# log all detected folders in console
message(STATUS "Folders Detected:")
foreach(dir IN LISTS FOLDERS)
    message(STATUS "${dir}")
endforeach()

# link all files to exec
add_executable(
    main
    ${FILES}
)

# inlcude all directories
# suppose a header file in src/utils/Helpers.hpp
# since all directories are included; you can run
# #include "Helpers.hpp" instead of #include "utils/Helpers.hpp"; utils/Helpers.hpp since src firectory has to be included
target_include_directories(
    main PRIVATE
    ${FOLDERS}
)

# CHANGE: link installed libraries here
target_link_libraries(
    main PRIVATE
    fmt::fmt
)
# END CHANGE
