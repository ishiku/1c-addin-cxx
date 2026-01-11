cmake_minimum_required(VERSION 3.24)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(c12cxx
    VERSION 1.0.0
    DESCRIPTION "Template for C++ library built with CMake"
    LANGUAGES CXX)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------

include(cmake/utils.cmake)
include(GNUInstallDirs)
include(cmake/ClangFormat.cmake)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

option(C12CXX_TIDY "run clang-tidy" ON)
option(C12CXX_BUILD_PIC "Build position independent code (-fPIC)" ON)

option(C12CXX_BUILD_TESTS "Build c12cxx tests" OFF)
option(C12CXX_BUILD_EXAMPLES "Build c12cxx examples" OFF)
option(C12CXX_BUILD_DOCS "Build c12cxx documentation" OFF)
option(C12CXX_INSTALL "Generate target for installing c12cxx" ${is_top_level})
set_if_undefined(C12CXX_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/c12cxx" CACHE STRING
    "Install path for c12cxx package-related CMake files")

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(C12CXX_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "Enabled clang-tidy")
endif()

if(SPDLOG_BUILD_PIC)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(c12cxx) # initialized below
add_library(c12cxx::c12cxx ALIAS c12cxx)

#----------------------------------------------------------------------------------------------------------------------
# c12cxx dependencies
#----------------------------------------------------------------------------------------------------------------------

# Search for your dependencies here

#----------------------------------------------------------------------------------------------------------------------
# c12cxx sources
#----------------------------------------------------------------------------------------------------------------------

set(sources
    include/c12cxx/details/Component.h
    include/c12cxx/details/function_traits.h
    include/c12cxx/details/Metadata.h
    include/c12cxx/details/Method.h 
    include/c12cxx/details/MethodWrapper.h
    include/c12cxx/details/Property.h
    include/c12cxx/details/ValueAccessor.h              
    src/dllmain.cpp
    src/isocalendar.cpp
    src/utfutils.cpp
    src/Component.cpp
    src/c12cxx.cpp
    src/exports.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# c12cxx target
#----------------------------------------------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

target_sources(c12cxx PRIVATE ${sources})

target_include_directories(c12cxx
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    PRIVATE
        src)

set_target_properties(c12cxx PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}    
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    )


if(C12CXX_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    configure_package_config_file(cmake/c12cxx-config.cmake.in c12cxx-config.cmake
        INSTALL_DESTINATION "${C12CXX_INSTALL_CMAKEDIR}")

    write_basic_package_version_file(c12cxx-config-version.cmake
        COMPATIBILITY SameMajorVersion)

    install(TARGETS c12cxx EXPORT c12cxx_export
        RUNTIME COMPONENT c12cxx
        LIBRARY COMPONENT c12cxx NAMELINK_COMPONENT c12cxx-dev
        ARCHIVE COMPONENT c12cxx-dev
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(DIRECTORY include/
        TYPE INCLUDE
        COMPONENT c12cxx-dev)

    set(targets_file "c12cxx-static-targets.cmake")

    install(EXPORT c12cxx_export
        COMPONENT c12cxx-dev
        FILE "${targets_file}"
        DESTINATION "${C12CXX_INSTALL_CMAKEDIR}"
        NAMESPACE c12cxx::)

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/c12cxx-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/c12cxx-config-version.cmake"
        COMPONENT c12cxx-dev
        DESTINATION "${C12CXX_INSTALL_CMAKEDIR}")

    if(MSVC)
        set(pdb_file "")
        set(pdb_file_destination "")

        # TARGET_PDB_FILE does not work for pdb file generated for static library build, determining it manually
        set(pdb_file "$<TARGET_FILE_DIR:c12cxx>/$<TARGET_FILE_PREFIX:c12cxx>$<TARGET_FILE_BASE_NAME:c12cxx>.pdb")
        set(pdb_file_destination "${CMAKE_INSTALL_LIBDIR}")

        install(FILES "${pdb_file}"
            COMPONENT c12cxx-dev
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION "${pdb_file_destination}"
            OPTIONAL)
    endif()
endif()

#----------------------------------------------------------------------------------------------------------------------
# other targets
#----------------------------------------------------------------------------------------------------------------------

if(C12CXX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(C12CXX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(C12CXX_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    doxygen_add_docs(docs include)
endif()