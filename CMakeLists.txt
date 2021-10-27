project(projeto_cg)

add_executable(${PROJECT_NAME} main.cpp openglwindow.cpp asteroids.cpp ship.cpp starlayers.cpp)

enable_abcg(${PROJECT_NAME})
