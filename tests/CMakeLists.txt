include(CTest)
find_package(GTest CONFIG REQUIRED)

add_executable(TestPath TestPath.cpp)
target_link_libraries(TestPath PRIVATE Common GTest::gtest GTest::gtest_main)
add_test(NAME TestPath COMMAND TestPath)

add_executable(PositionAndDirTests PositionAndDirTests.cpp)
target_link_libraries(PositionAndDirTests PRIVATE Common GTest::gtest GTest::gtest_main)
add_test(NAME PositionAndDirTests COMMAND PositionAndDirTests)

add_executable(SmallVectorTests SmallVectorTests.cpp)
target_link_libraries(SmallVectorTests PRIVATE Common GTest::gtest GTest::gtest_main)
add_test(NAME SmallVectorTests COMMAND SmallVectorTests)
