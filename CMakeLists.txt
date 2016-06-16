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
add_definitions(${toolchain_PKGCONFIG_CFLAGS_OTHER} -ggdb -Wall -pedantic)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# add the executables
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
    genBG c_bagel
    )
add_dependencies(bg-generate
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
add_executable(map
    src/mapper.c
    )
target_link_libraries(map
    mapBG hwGraph c_bagel
    )
add_dependencies(map
    mapBG hwGraph c_bagel
    )
add_executable(map-transform
    src/generateBGFromMapping.c
    )
target_link_libraries(map-transform
    mapBG hwGraph c_bagel
    )
add_dependencies(map-transform
    mapBG hwGraph c_bagel
    )
#add_executable(deploy
    #src/deployer.c
    #)
#target_link_libraries(deploy
    #mapBG hwGraph c_bagel
    #)
#add_dependencies(deploy
    #mapBG hwGraph c_bagel
    #)
add_executable(map-route
    src/generateRoutingInfo.c
    )
target_link_libraries(map-route
    mapBG hwGraph c_bagel genBG
    )
add_dependencies(map-route
    mapBG hwGraph c_bagel genBG
    )

# add custom targets (symlinks)
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
