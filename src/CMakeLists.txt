add_subdirectory(Common)
add_subdirectory(Game)
add_subdirectory(CursesIO)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE CursesIO)