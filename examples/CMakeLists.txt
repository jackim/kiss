project(echo)
add_executable(echo echo.cpp)
target_link_libraries(echo kiss pthread)

