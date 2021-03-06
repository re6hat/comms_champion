# - Find CommsChampion
# Find the CommsChampion includes and libraries
# 
# Output is provided using the following variables
#  CC_COMMS_FOUND - Found COMMS library, use CC_INCLUDE_DIRS for includes
#  CC_COMMS_CHAMPION_FOUND - Found CommsChampion, use CC_INCLUDE_DIRS, CC_PLUGIN_LIBRARIES, CC_PLUGIN_DIR)
#  CC_INCLUDE_DIRS - Where to find all the headers
#  CC_PLUGIN_LIBRARIES - Libraries to link against when building a plugin
#  CC_PLUGIN_LIBRARY_DIRS - Where to find the libraries required to build plugin.
#  CC_PLUGIN_DIR - Directory where to install newly built plugin

get_filename_component (CC_INSTALL_PATH ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

find_path(CC_INCLUDE_DIR NAMES comms/comms.h PATHS "${CC_INSTALL_PATH}" PATH_SUFFIXES include)
find_library(CC_PLUGIN_LIBRARY  NAMES "comms_champion" PATHS "${CC_INSTALL_PATH}" PATH_SUFFIXES lib)

if (MSVC)
    find_file(CC_NULL_SOCK_LIBRARY NAMES "null_socket.dll" PATHS "${CC_INSTALL_PATH}" PATH_SUFFIXES plugin)
else ()
    find_library(CC_NULL_SOCK_LIBRARY  NAMES "null_socket" PATHS "${CC_INSTALL_PATH}" PATH_SUFFIXES plugin)
endif ()

if (CC_PLUGIN_LIBRARY)
    get_filename_component  (CC_PLUGIN_LIBRARY_DIR ${CC_PLUGIN_LIBRARY} DIRECTORY)
endif ()

if (CC_NULL_SOCK_LIBRARY)
    get_filename_component  (CC_PLUGIN_DIR ${CC_NULL_SOCK_LIBRARY} DIRECTORY)
endif ()

find_package(PackageHandleStandardArgs REQUIRED)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CC_COMMS REQUIRED_VARS CC_INCLUDE_DIR)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CC_COMMS_CHAMPION REQUIRED_VARS 
    CC_INCLUDE_DIR CC_PLUGIN_LIBRARY_DIR CC_PLUGIN_LIBRARY CC_PLUGIN_DIR)
    
if (CC_COMMS_FOUND)
    set (CC_INCLUDE_DIRS ${CC_INCLUDE_DIR})
endif ()

if (CC_COMMS_CHAMPION_FOUND)
    set (CC_PLUGIN_LIBRARIES ${CC_PLUGIN_LIBRARY})
    set (CC_PLUGIN_LIBRARY_DIRS ${CC_PLUGIN_LIBRARY_DIR})
endif ()    

