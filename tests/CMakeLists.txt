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

add_executable(Static2DArrTests Static2DArrTests.cpp)
target_link_libraries(Static2DArrTests PRIVATE Common GTest::gtest GTest::gtest_main)
add_test(NAME Static2DArrTests COMMAND Static2DArrTests)

add_executable(CompileTimeHashMapTests CompileTimeHashMapTests.cpp)
target_link_libraries(CompileTimeHashMapTests PRIVATE Common GTest::gtest GTest::gtest_main)
add_test(NAME CompileTimeHashMapTests COMMAND CompileTimeHashMapTests)

add_executable(ObjectContainerTests ObjectContainerTests.cpp)
target_link_libraries(ObjectContainerTests PRIVATE Game GTest::gtest GTest::gtest_main)
add_test(NAME ObjectContainerTests COMMAND ObjectContainerTests)

add_executable(DiceTests DiceTests.cpp)
target_link_libraries(DiceTests PRIVATE Common GTest::gtest GTest::gtest_main)
add_test(NAME DiceTests COMMAND DiceTests)

add_executable(SerializationLibTests SerializationLibTests.cpp)
target_link_libraries(SerializationLibTests PRIVATE SerialzationLib GTest::gtest GTest::gtest_main)
add_test(NAME SerializationLibTests COMMAND SerializationLibTests)
