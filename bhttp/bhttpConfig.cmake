include(CMakeFindDependencyMacro)
find_dependency(pyu 1.0.0 REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/bhttp.cmake)