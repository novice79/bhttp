cmake_minimum_required(VERSION 3.20)

project(bhttp LANGUAGES C CXX VERSION 1.0.0)

file(GLOB HEADER_FILES "*.hpp")
install(FILES ${HEADER_FILES} DESTINATION "include/bhttp")
