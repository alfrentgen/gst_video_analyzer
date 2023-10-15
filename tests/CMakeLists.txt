add_executable(showcase "showcase.cpp")
target_include_directories(showcase PRIVATE ${GSTREAMER_INCLUDE_DIRS} ${GST_APP_INCLUDE_DIRS})
target_link_libraries(showcase ${GSTREAMER_LIBRARIES} ${GST_APP_LIBRARIES})
