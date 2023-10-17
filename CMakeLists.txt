cmake_minimum_required(VERSION 3.13)
project(video_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR})
pkg_search_module(GLIB REQUIRED glib-2.0) 
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB HDR ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(TARGET_NAME video_analyzer)

add_library(${TARGET_NAME} SHARED ${SRC} ${HDR})
target_include_directories(${TARGET_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS} ${GSTREAMER_INCLUDE_DIRS} ${GST_APP_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} ${OpenCV_LIBRARIES} ${GST_VIDEO_LIBRARIES}  ${GST_APP_LIBRARIES})

add_subdirectory(tests)
