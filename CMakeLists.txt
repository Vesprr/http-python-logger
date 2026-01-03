cmake_minimum_required(VERSION 4.0)
project(http-logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow CONFIG REQUIRED)

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

option(BUILD_STATIC_LIB "Build static library instead of executable" OFF)
set(NAME main)

if(BUILD_STATIC_LIB)
    # if static lib, name = project name
    set(NAME ${PROJECT_NAME})
    # link all files to exec
    add_library(
        ${NAME}
        STATIC
        ${FILES}
        "${CMAKE_SOURCE_DIR}/embedded/embeds.hpp"
    )

    # remove prefix
    set_target_properties(${NAME} PROPERTIES PREFIX "")

    target_include_directories(${NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
else()
    # link all files to exec
    add_executable(
        ${NAME}
        ${FILES}
        "${CMAKE_SOURCE_DIR}/embedded/embeds.hpp"
    )
endif()

# inlcude all directories
# suppose a header file in src/utils/Helpers.hpp
# since all directories are included; you can run
# #include "Helpers.hpp" instead of #include "utils/Helpers.hpp"; utils/Helpers.hpp since src firectory has to be included
target_include_directories(
    ${NAME} PRIVATE
    ${FOLDERS}
    "${CMAKE_SOURCE_DIR}/external/included"
    "${CMAKE_SOURCE_DIR}/embedded"
    "${CMAKE_SOURCE_DIR}/embedded/incs"
)
# files in external/included are also included to allow downloading libraries and using them

# CHANGE: link installed libraries here
target_link_libraries(
    ${NAME} PRIVATE
    Crow::Crow asio::asio
)
# END CHANGE
