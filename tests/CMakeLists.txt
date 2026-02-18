add_executable(TestPath TestPath.cpp)
target_link_options(TestPath PUBLIC -fsanitize=undefined)
target_link_libraries(TestPath PUBLIC Common)

include(CTest)


add_test(NAME PathTest COMMAND TestPath)