cmake_minimum_required(VERSION 3.10)
project(MyLoggerProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(my_logger
    main.cpp
    logger.cpp
    other_process.cpp
)

# You can also list headers for IDEs
target_include_directories(my_logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
