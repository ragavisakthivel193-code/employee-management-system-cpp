cmake_minimum_required(VERSION 3.15)
project(EmployeeManagementSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    "C:/Program Files/MySQL/MySQL Server 8.0/include"
)

# Library paths
link_directories(
    "C:/Program Files/MySQL/MySQL Server 8.0/lib"
)

# Source files
set(SOURCES
    src/main.cpp
    src/Employee.cpp
    src/Database.cpp
    src/Auth.cpp
    src/EmployeeService.cpp
)

# Executable target
add_executable(EmployeeManagementSystem ${SOURCES})

# Platform-specific linking
if(WIN32)
    target_link_libraries(EmployeeManagementSystem PRIVATE
        libmysql
        ws2_32
        crypt32
    )

    # Automatically copy libmysql.dll to output folder
    add_custom_command(TARGET EmployeeManagementSystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Program Files/MySQL/MySQL Server 8.0/lib/libmysql.dll"
        $<TARGET_FILE_DIR:EmployeeManagementSystem>
    )
else()
    target_link_libraries(EmployeeManagementSystem PRIVATE
        mysqlclient
        pthread
    )
endif()
