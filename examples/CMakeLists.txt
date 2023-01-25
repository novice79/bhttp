cmake_minimum_required(VERSION 3.20)

project(test)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
# use all static lib
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif()
# include_directories(${CMAKE_SOURCE_DIR})
file(GLOB_RECURSE SOURCE_FILES  "*.cpp")
foreach(file ${SOURCE_FILES})
    # DIRECTORY = Directory without file name
    # NAME      = File name without directory
    # EXT       = File name longest extension (.b.c from d/a.b.c)
    # NAME_WE   = File name with neither the directory nor the longest extension
    # LAST_EXT  = File name last extension (.c from d/a.b.c)
    # NAME_WLE  = File name with neither the directory nor the last extension
    get_filename_component(name "${file}" NAME_WE)
    add_executable(${name} "${file}")
    list(APPEND targets ${name})
endforeach()

# message(FATAL_ERROR "You can not do this at all, CMake will exit." )

# set(Boost_DEBUG ON)
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
# set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.79.0 REQUIRED COMPONENTS 
    date_time 
    filesystem 
    serialization
    atomic 
    json
    log_setup 
    log 
    program_options
    system 
    locale 
)

# set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL 1.1.1 REQUIRED)
message("OPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}")
message("OPENSSL_LIBRARIES=${OPENSSL_LIBRARIES}")

# sudo apt-get install libz-dev
# find_package(ZLIB REQUIRED)
# message("ZLIB_LIBRARIES=${ZLIB_LIBRARIES}")

find_path(bhttp_headers "bhttp/server_http.hpp")
if(NOT bhttp_headers)
  message(FATAL_ERROR "Can't find folder containing bhttp/server_http.hpp")
endif()
include_directories(${bhttp_headers})
foreach(t ${targets})
    target_link_libraries(
        ${t} 
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        # ${ZLIB_LIBRARIES}
        # ZLIB::ZLIB
        ${CMAKE_DL_LIBS} 
    )
    if(WIN32)
        target_link_libraries(${t} ws2_32 wsock32)
    endif()
    install(TARGETS ${t} RUNTIME DESTINATION bin)
endforeach()




