project(echo)
add_executable(echo echo.cpp)
target_link_libraries(echo kiss pthread)

project(httpserver)
add_executable(httpserver httpserver.cpp)
target_link_libraries(httpserver kiss pthread)

project(websocketserver)
add_executable(websocketserver websocketserver.cpp)
target_link_libraries(websocketserver kiss pthread)

