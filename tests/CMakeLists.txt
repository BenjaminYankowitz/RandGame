add_executable(TestPath TestPath.cpp)
target_link_options(TestPath PUBLIC -fsanitize=undefined)
target_link_libraries(TestPath PUBLIC Common)

add_executable(ConstexprTests ConstexprTests.cpp)
target_link_options(ConstexprTests PUBLIC -fsanitize=undefined)
target_link_libraries(ConstexprTests PUBLIC Common)

add_test(NAME PathTest COMMAND TestPath)
add_test(NAME ConstexprTests COMMAND ConstexprTests)