cmake_minimum_required(VERSION 3.20)

project(bhttp LANGUAGES C CXX VERSION 1.0.0)

file(GLOB HEADER_FILES "*.hpp")
file(GLOB_RECURSE SOURCE_FILES 
    "*.cpp"
)
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.79.0 REQUIRED COMPONENTS 
    date_time 
    filesystem 
    serialization
    atomic 
    json
    log_setup 
    log 
    system 
)
# include_directories(${Boost_INCLUDE_DIRS})
target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC
    ${Boost_INCLUDE_DIRS}
)
find_package(OpenSSL 1.1.1 REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(
    ${PROJECT_NAME} 
    PUBLIC
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
find_package(ZLIB REQUIRED)
find_package(pyu 1.0.0 REQUIRED)
get_target_property(pyu_INCLUDE_DIRS pyu::pyu INTERFACE_INCLUDE_DIRECTORIES )
message("pyu_INCLUDE_DIRS=${pyu_INCLUDE_DIRS}")
get_target_property(pyu_LIBRARIES pyu::pyu IMPORTED_LOCATION_RELEASE )
message("pyu_LIBRARIES=${pyu_LIBRARIES}")
message("pyu_VERSION=${pyu_VERSION}")
include_directories(${pyu_INCLUDE_DIRS})
target_link_libraries(
    ${PROJECT_NAME} 
    PUBLIC
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    pyu::pyu
)
install(
    TARGETS ${PROJECT_NAME} 
    EXPORT bhttp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    PUBLIC_HEADER DESTINATION "include/bhttp"
)
install(EXPORT bhttp
    FILE bhttp.cmake
    NAMESPACE bhttp::
    DESTINATION lib/cmake/bhttp
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "bhttpConfigVersion.cmake"
    VERSION ${bhttp_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES 
    "bhttpConfig.cmake" 
    "${CMAKE_CURRENT_BINARY_DIR}/bhttpConfigVersion.cmake"
    DESTINATION lib/cmake/bhttp
)


