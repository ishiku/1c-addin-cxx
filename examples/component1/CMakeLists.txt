cmake_minimum_required(VERSION 3.24)

project(component1
    LANGUAGES CXX)

include("../../cmake/utils.cmake")
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

if(is_top_level)
    find_package(c12cxx REQUIRED)
endif()


add_library(component1 SHARED src/Component1.cpp)
target_link_libraries(component1 PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,c12cxx::c12cxx>" )


set_target_properties(component1 PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-m64)
    add_link_options(-m64)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()
    

if(NOT is_top_level)
    win_copy_deps_to_target_dir(component1 c12cxx::c12cxx)
endif()




