cmake_minimum_required(VERSION 3.21)

PROJECT(dmrapidpool)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(cmake/ModuleImport.cmake)
INCLUDE(cmake/ModuleCompileOptions.cmake)
ModuleSetCompileOptions()

ADD_DEFINITIONS(-DUSE_DMRAPIDPOOL)

InterfaceImport("dmrapidpool" "include" "")

if(PROJECT_IS_TOP_LEVEL)
    ExeImport("test" "dmrapidpool")
endif()
