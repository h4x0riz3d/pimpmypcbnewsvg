cmake_minimum_required(VERSION 2.8)
project("pimpmypcbnewsvg")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 11)
find_package(TinyXML2)


include_directories(${TINYXML2_INCLUDE_DIR})
add_definitions(-D__CMAKE__)
add_executable(pimpmypcbnewsvg pimpmypcbnewsvg.cpp)
target_link_libraries(pimpmypcbnewsvg ${TINYXML2_LIBRARIES})
install(TARGETS pimpmypcbnewsvg DESTINATION ${CMAKE_SOURCE_DIR})