include(CTest)

add_executable(TestPath TestPath.cpp)
target_link_options(TestPath PUBLIC -fsanitize=undefined)
target_link_libraries(TestPath PRIVATE Common)

add_test(NAME TestPath COMMAND TestPath)


add_executable(ConstexprTests ConstexprTests.cpp)
target_link_options(ConstexprTests PUBLIC -fsanitize=undefined)
target_link_libraries(ConstexprTests PRIVATE Common)

add_test(NAME ConstexprTests COMMAND ConstexprTests)