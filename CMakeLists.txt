cmake_minimum_required(VERSION 2.8)

project(toolchain)

include_directories(src)
#include_directories(include)

# they are rebuilt completely from scratch inside our build-tree
# each subdir is a "project", so some standard variables are defined therein (like protocol_SOURCE_DIR for example)
# but we ourself may be add_subdirectory'ed, with some of the targets already present. so check this!

# This is not possible without mars cmake file :/ So we have to compile this on our own
if(NOT TARGET c_bagel)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ext/c_bagel c_bagel)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ext/c_bagel/src)
endif(NOT TARGET c_bagel)
if(NOT TARGET hwGraph)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../hwGraph hwGraph)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../hwGraph/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../hwGraph/src)
endif(NOT TARGET hwGraph)
if(NOT TARGET mapBG)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../mapBG mapBG)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mapBG/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mapBG/src)
endif(NOT TARGET mapBG)
if(NOT TARGET genBG)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../genBG genBG)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../genBG/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../genBG/src)
endif(NOT TARGET genBG)

SET(ENV{PKG_CONFIG_PATH}
    ${CMAKE_BINARY_DIR}:$ENV{PKG_CONFIG_PATH}
    )

find_package(PkgConfig)
pkg_check_modules(toolchain_PKGCONFIG REQUIRED
    c_bagel hwGraph mapBG genBG
    )

include_directories(${toolchain_PKGCONFIG_INCLUDE_DIRS})
message(${toolchain_PKGCONFIG_INCLUDE_DIRS})
link_directories(${toolchain_PKGCONFIG_LIBRARY_DIRS})
add_definitions(${toolchain_PKGCONFIG_CFLAGS_OTHER} -ggdb -Wall -Werror -pedantic)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# add the executables
add_executable(bg2dict
    src/generateBG.c
    )
target_link_libraries(bg2dict
    genBG c_bagel
    )
add_dependencies(bg2dict
    genBG c_bagel
    )
add_executable(fill-template
    src/generateFromTemplate.c
    )
target_link_libraries(fill-template
    genBG
    )
add_dependencies(fill-template
    genBG
    )
add_executable(sw2hw
    src/mapper.c
    )
target_link_libraries(sw2hw
    mapBG hwGraph c_bagel
    )
add_dependencies(sw2hw
    mapBG hwGraph c_bagel
    )
add_executable(map2bg
    src/generateBGFromMapping.c
    )
target_link_libraries(map2bg
    mapBG hwGraph c_bagel
    )
add_dependencies(map2bg
    mapBG hwGraph c_bagel
    )

# add custom targets (symlinks)
add_custom_target(bg2dict-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/bg2dict ${CMAKE_SOURCE_DIR}/bg2dict
    DEPENDS bg2dict)
add_custom_target(fill-template-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/fill-template ${CMAKE_SOURCE_DIR}/fill-template
    DEPENDS fill-template)
add_custom_target(sw2hw-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/sw2hw ${CMAKE_SOURCE_DIR}/sw2hw
    DEPENDS sw2hw)
add_custom_target(map2bg-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/map2bg ${CMAKE_SOURCE_DIR}/map2bg
    DEPENDS map2bg)
# and clean them up
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/bg2dict")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/fill-template")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/sw2hw")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/map2bg")
