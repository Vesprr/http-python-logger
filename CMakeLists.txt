cmake_minimum_required(VERSION 3.26)
project(http-logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow CONFIG REQUIRED)

option(BUILD_STATIC_LIB "Build static library instead of executable" OFF)

# get CPP files as sources
# files in driver will be ignored later on
file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/driver/*.cpp
)

if(BUILD_STATIC_LIB)
    set(TARGET_NAME ${PROJECT_NAME})
    # add static lib target
    add_library(${TARGET_NAME} STATIC ${PROJECT_SOURCES})
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
else()
    set(TARGET_NAME driver_main)
    # add executable target
    add_executable(${TARGET_NAME} ${PROJECT_SOURCES})
endif()

# include dirs
target_include_directories(${TARGET_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/included
    ${CMAKE_SOURCE_DIR}/embedded
)

# link libs
target_link_libraries(${TARGET_NAME}
    PRIVATE
    Crow::Crow
    asio::asio
)

if(BUILD_STATIC_LIB)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install library and headers
    install(
        TARGETS ${TARGET_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Export targets
    install(
        EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    # Generate config files
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endif()
