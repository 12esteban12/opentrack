find_package(OpenCV 3.0 QUIET)
set(modules opencv_core opencv_videoio opencv_imgproc)
if(OpenCV_FOUND)
    otr_module(tracker-pt-base STATIC)
    target_include_directories(opentrack-tracker-pt-base SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(opentrack-tracker-pt-base opentrack-cv ${modules})
endif()
add_subdirectory(module)

