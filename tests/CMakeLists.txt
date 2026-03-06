include(CTest)

add_executable(TestPath TestPath.cpp)
target_link_libraries(TestPath PRIVATE Common)

add_test(NAME TestPath COMMAND TestPath)


add_executable(ConstexprTests ConstexprTests.cpp)
target_link_libraries(ConstexprTests PRIVATE Common)

add_test(NAME ConstexprTests COMMAND ConstexprTests)