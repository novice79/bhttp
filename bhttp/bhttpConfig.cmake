include(CMakeFindDependencyMacro)
find_dependency(Boost 1.79.0 REQUIRED)
find_dependency(OpenSSL 1.1.1 REQUIRED)
find_dependency(ZLIB REQUIRED)
find_dependency(pyu 1.0.0 REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/bhttp.cmake)