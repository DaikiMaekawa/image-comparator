cmake_minimum_required(VERSION 2.8)

project(ImageComparator)

find_package(OpenCV)

include_directories(
    ${PROJECT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(ImageComparator_SRCS
    main.cpp
)

add_executable(ImageComparator
    ${ImageComparator_SRCS}
)

target_link_libraries(ImageComparator
    ${OpenCV_LIBS}
)
    
