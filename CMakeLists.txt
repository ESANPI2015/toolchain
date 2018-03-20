cmake_minimum_required(VERSION 2.8)

project(toolchain)

include_directories(src)
#include_directories(include)

# they are rebuilt completely from scratch inside our build-tree
# each subdir is a "project", so some standard variables are defined therein (like protocol_SOURCE_DIR for example)
# but we ourself may be add_subdirectory'ed, with some of the targets already present. so check this!

# This is not possible without mars cmake file :/ So we have to compile this on our own
if(NOT TARGET hypergraph)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hypergraph hypergraph)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hypergraph/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hypergraph/src)
endif(NOT TARGET hypergraph)
if(NOT TARGET domaingraph)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/domaingraph domaingraph)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/domaingraph/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/domaingraph/src)
endif(NOT TARGET domaingraph)
if(NOT TARGET hwgraph)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hwgraph hwgraph)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hwgraph/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/hwgraph/src)
endif(NOT TARGET hwgraph)
if(NOT TARGET swgraph)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/swgraph swgraph)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/swgraph/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/swgraph/src)
endif(NOT TARGET swgraph)
if(NOT TARGET HypergraphGUI)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gui HypergraphGUI)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gui/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gui/src)
endif(NOT TARGET HypergraphGUI)

SET(ENV{PKG_CONFIG_PATH}
    ${CMAKE_BINARY_DIR}:$ENV{PKG_CONFIG_PATH}
    )

find_package(PkgConfig)
pkg_check_modules(toolchain_PKGCONFIG REQUIRED
    hypergraph domaingraph hwgraph swgraph
    )

include_directories(${toolchain_PKGCONFIG_INCLUDE_DIRS})
message(${toolchain_PKGCONFIG_INCLUDE_DIRS})
link_directories(${toolchain_PKGCONFIG_LIBRARY_DIRS})
add_definitions(${toolchain_PKGCONFIG_CFLAGS_OTHER} -Wall -pedantic)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# add the executables
#add_executable(npi-compose
#    src/analyseBG.c
#    )
#target_link_libraries(bg-analyse
#    c_bagel
#    )
#add_dependencies(bg-analyse
#    c_bagel
#    )

# add custom targets (symlinks)
add_custom_target(simplify-graph-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/hypergraph/simplify ${CMAKE_SOURCE_DIR}/simplify-graph
    DEPENDS hypergraph)
add_custom_target(dbg-import-hw-model-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/hwgraph/dbg-import-hw-model ${CMAKE_SOURCE_DIR}/dbg-import-hw-model
    DEPENDS hwgraph)
add_custom_target(dbg-export-hw-model-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/hwgraph/dbg-export-hw-model ${CMAKE_SOURCE_DIR}/dbg-export-hw-model
    DEPENDS hwgraph)
add_custom_target(bg-import-model-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/swgraph/bg-import-model ${CMAKE_SOURCE_DIR}/bg-import-model
    DEPENDS swgraph)
add_custom_target(bg-export-model-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/swgraph/bg-export-model ${CMAKE_SOURCE_DIR}/bg-export-model
    DEPENDS swgraph)
add_custom_target(hypergraph-gui-lazy-symlink ALL
    ln --force -s ${CMAKE_CURRENT_BINARY_DIR}/HypergraphGUI/HypergraphGUI ${CMAKE_SOURCE_DIR}/hypergraph-gui
    DEPENDS HypergraphGUI)
# and clean them up
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/simplify-graph")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/hypergraph-gui")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/dbg-import-hw-model")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/dbg-export-hw-model")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/bg-import-model")
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/bg-export-model")
