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
if(NOT TARGET hwgraph)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hwgraph hwgraph)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hwgraph/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hwgraph/src)
endif(NOT TARGET hwgraph)
if(NOT TARGET mapbg)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/mapbg mapbg)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/mapbg/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/mapbg/src)
endif(NOT TARGET mapbg)
if(NOT TARGET genbg)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/genbg genbg)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/genbg/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/genbg/src)
endif(NOT TARGET genbg)
if(NOT TARGET ndlcom)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/ndlcom ndlcom)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/ndlcom/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/ndlcom/src)
endif(NOT TARGET ndlcom)
if(NOT TARGET representations)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/representations representations)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/representations/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/representations/src)
endif(NOT TARGET representations)
if(NOT TARGET isp)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/isp isp)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/isp/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/isp/src)
endif(NOT TARGET isp)

SET(ENV{PKG_CONFIG_PATH}
    ${CMAKE_BINARY_DIR}:$ENV{PKG_CONFIG_PATH}
    )

find_package(PkgConfig)
pkg_check_modules(toolchain_PKGCONFIG REQUIRED
    c_bagel hwgraph mapbg genbg ndlcom representations isp
    )

include_directories(${toolchain_PKGCONFIG_INCLUDE_DIRS})
message(${toolchain_PKGCONFIG_INCLUDE_DIRS})
link_directories(${toolchain_PKGCONFIG_LIBRARY_DIRS})
add_definitions(${toolchain_PKGCONFIG_CFLAGS_OTHER} -ggdb -Wall -pedantic)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# add the executables
#add_executable(isp
    #src/isp.cpp
    #)
#target_link_libraries(isp
    #ndlcom representations
    #)
#add_dependencies(isp
    #ndlcom representations
    #)
add_executable(bg-analyse
    src/analyseBG.c
    )
target_link_libraries(bg-analyse
    c_bagel
    )
add_dependencies(bg-analyse
    c_bagel
    )
add_executable(bg-generate
    src/generateBG.c
    )
target_link_libraries(bg-generate
    genbg c_bagel
    )
add_dependencies(bg-generate
    genbg c_bagel
    )
add_executable(fill-template
    src/generateFromTemplate.c
    )
target_link_libraries(fill-template
    genbg
    )
add_dependencies(fill-template
    genbg
    )
add_executable(map
    src/mapper.c
    )
target_link_libraries(map
    mapbg hwgraph c_bagel
    )
add_dependencies(map
    mapbg hwgraph c_bagel
    )
add_executable(map-transform
    src/generateBGFromMapping.c
    )
target_link_libraries(map-transform
    mapbg hwgraph c_bagel
    )
add_dependencies(map-transform
    mapbg hwgraph c_bagel
    )
#add_executable(deploy
    #src/deployer.c
    #)
#target_link_libraries(deploy
    #mapbg hwgraph c_bagel
    #)
#add_dependencies(deploy
    #mapbg hwgraph c_bagel
    #)
add_executable(map-route
    src/generateRoutingInfo.c
    )
target_link_libraries(map-route
    mapbg hwgraph c_bagel genbg
    )
add_dependencies(map-route
    mapbg hwgraph c_bagel genbg
    )

# add custom targets (symlinks)
add_custom_target(isp-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/isp/tools/isprog ${CMAKE_SOURCE_DIR}/isp
    DEPENDS isp)
add_custom_target(bg-analyse-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/bg-analyse ${CMAKE_SOURCE_DIR}/bg-analyse
    DEPENDS bg-analyse)
add_custom_target(bg-generate-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/bg-generate ${CMAKE_SOURCE_DIR}/bg-generate
    DEPENDS bg-generate)
add_custom_target(fill-template-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/fill-template ${CMAKE_SOURCE_DIR}/fill-template
    DEPENDS fill-template)
add_custom_target(map-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/map ${CMAKE_SOURCE_DIR}/map
    DEPENDS map)
add_custom_target(map-transform-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/map-transform ${CMAKE_SOURCE_DIR}/map-transform
    DEPENDS map-transform)
#add_custom_target(deploy-lazy-symlink ALL
    #ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/deploy ${CMAKE_SOURCE_DIR}/deploy
    #DEPENDS deploy)
add_custom_target(map-route-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/map-route ${CMAKE_SOURCE_DIR}/map-route
    DEPENDS map-route)
# and clean them up
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/isp")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/bg-analyse")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/bg-generate")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/fill-template")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/map")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/map-transform")
#set_directory_properties(PROPERTIES
    #ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/deploy")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/map-route")
