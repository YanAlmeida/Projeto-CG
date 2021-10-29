project(projeto_cg)

add_executable(${PROJECT_NAME} main.cpp openglwindow.cpp asteroids.cpp cat.cpp clouds.cpp starlayers.cpp)

enable_abcg(${PROJECT_NAME})
