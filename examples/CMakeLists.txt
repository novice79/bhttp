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
    add_executable(${name} "${file}" "android/miss_unwind.c")
    list(APPEND targets ${name})
    if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
        set_target_properties(${name} PROPERTIES
            BUNDLE True
            MACOSX_BUNDLE_GUI_IDENTIFIER piaoyun.dev.${name}
            MACOSX_BUNDLE_BUNDLE_NAME ${name}
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/../misc/customtemplate.plist.in
        )
        # This little macro lets you set any XCode specific property
        macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
            set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
        endmacro (set_xcode_property)
        macro (unset_xcode_property TARGET XCODE_PROPERTY)
        set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY})
        endmacro (unset_xcode_property)
        set_xcode_property(${name} CODE_SIGN_IDENTITY "iPhone Developer")
        # grep DEVELOPMENT_TEAM _build_ios/main.xcodeproj/project.pbxproj
        set_xcode_property(${name} DEVELOPMENT_TEAM W8L2TKP6RD)
    endif()
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
    system 
)

# set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL 1.1.1 REQUIRED)
message("OPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}")
message("OPENSSL_LIBRARIES=${OPENSSL_LIBRARIES}")

# sudo apt-get install libz-dev
# find_package(ZLIB REQUIRED)
# message("ZLIB_LIBRARIES=${ZLIB_LIBRARIES}")
find_package(pyu 1.0.0 REQUIRED)
get_target_property(pyu_INCLUDE_DIRS pyu::pyu INTERFACE_INCLUDE_DIRECTORIES )
message("pyu_INCLUDE_DIRS=${pyu_INCLUDE_DIRS}")
get_target_property(pyu_LIBRARIES pyu::pyu IMPORTED_LOCATION_RELEASE )
message("pyu_LIBRARIES=${pyu_LIBRARIES}")
message("pyu_VERSION=${pyu_VERSION}")
include_directories(${pyu_INCLUDE_DIRS})

find_package(bhttp REQUIRED)
get_target_property(bhttp_INCLUDE_DIRS bhttp::bhttp INTERFACE_INCLUDE_DIRECTORIES )
message("bhttp_INCLUDE_DIRS=${bhttp_INCLUDE_DIRS}")
get_target_property(bhttp_LIBRARIES bhttp::bhttp IMPORTED_LOCATION_RELEASE )
message("bhttp_LIBRARIES=${bhttp_LIBRARIES}")
message("bhttp_VERSION=${bhttp_VERSION}")
include_directories(${bhttp_INCLUDE_DIRS})
foreach(t ${targets})
    target_link_libraries(
        ${t} 
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        # ${ZLIB_LIBRARIES}
        # ZLIB::ZLIB
        bhttp::bhttp
        ${LUA_LIBRARIES}
        ${CMAKE_DL_LIBS} 
    )
    if(WIN32)
        target_link_libraries(${t} ws2_32 wsock32)
    endif()
    if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
        install(TARGETS ${t} BUNDLE DESTINATION bin)
    else()
        install(TARGETS ${t} RUNTIME DESTINATION bin)
    endif()
endforeach()




