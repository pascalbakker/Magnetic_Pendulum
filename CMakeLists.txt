cmake_minimum_required(VERSION 3.10)
project(MagneticPendulum C)

set(CMAKE_C_STANDARD 99)

# Include header files from the 'include' directory
include_directories(include)

# Add the executable target for MagneticPendulum
add_executable(MagneticPendulum main.c)

# Link the math library
target_link_libraries(MagneticPendulum m)
