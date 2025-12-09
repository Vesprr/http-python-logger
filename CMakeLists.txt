cmake_minimum_required(VERSION 4.0)
project(project-name LANGUAGES CXX)
# SET PROJECT NAME HERE

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt REQUIRED)
find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)

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
    "${CMAKE_SOURCE_DIR}/external/included"
)
# files in external/included are also included to allow downloading libraries and using them
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMENT "Copying asset files..."
)

add_dependencies(main copy_assets)

# CHANGE: link installed libraries here
target_link_libraries(
    main PRIVATE
    fmt::fmt
    SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System
)
# END CHANGE