add_executable(showcase "showcase.cpp")
target_include_directories(showcase PRIVATE ${OpenCV_INCLUDE_DIRS} ${GSTREAMER_INCLUDE_DIRS} ${GST_APP_INCLUDE_DIRS})
target_link_libraries(showcase ${OpenCV_LIBRARIES} ${GSTREAMER_LIBRARIES} ${GST_APP_LIBRARIES})
